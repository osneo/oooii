// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/leak_tracker.h>
#include <oBase/assert.h>
#include <oMemory/byte.h>
#include <oMemory/wang_hash.h>
#include <oCompiler.h>

using namespace std;

namespace ouro {

static inline uint64_t leak_tracker_hash(uintptr_t key)
{
	return wang_hash(key);
}

leak_tracker::size_type leak_tracker::calc_size(size_type capacity)
{
	return byte_align(concurrent_object_pool<entry>::calc_size(capacity)
		+ concurrent_hash_map::calc_size(capacity)
		, oCACHE_LINE_SIZE);
}

leak_tracker::leak_tracker()
{
	current_context.store(0);
}

leak_tracker::leak_tracker(const init_t& i, size_type capacity, const char* alloc_label, const allocator& a)
{
	initialize(i, capacity, alloc_label, a);
}

leak_tracker::leak_tracker(const init_t& i, void* memory, size_type capacity)
{
	initialize(i, memory, capacity);
}

leak_tracker::leak_tracker(leak_tracker&& that)
{
	allocs = std::move(that.allocs);
	pool = std::move(that.pool);
	delay_latch = std::move(that.delay_latch);
	current_context.store(that.current_context.load()); that.current_context = 0;
	init = that.init; that.init = init_t();
}

leak_tracker::~leak_tracker()
{
	deinitialize();
}

leak_tracker& leak_tracker::operator=(leak_tracker&& that)
{
	if (this != &that)
	{
		allocs = std::move(that.allocs);
		pool = std::move(that.pool);
		delay_latch = std::move(that.delay_latch);
		current_context.store(that.current_context.load()); that.current_context = 0;
		init = that.init; that.init = init_t();
	}
	return *this;
}

void leak_tracker::initialize(const init_t& i, size_type capacity, const char* alloc_label, const allocator& a)
{
	this->init = i;
	current_context.store(0);
	allocs.initialize(capacity, alloc_label, a);
	auto req = pool.calc_size(capacity);
	void* mem = a.allocate(req, memory_alignment::cacheline, alloc_label);
	pool.initialize(mem, capacity);
}

void leak_tracker::initialize(const init_t& i, void* memory, size_type capacity)
{
	this->init = i;
	current_context.store(0);
	allocs.initialize(memory, capacity);
	auto req = allocs.calc_size(capacity);
	void* mem = byte_align(byte_add(memory, req), oCACHE_LINE_SIZE);
	pool.initialize(mem, capacity);
}

void* leak_tracker::deinitialize()
{
	current_context.store(0);
	pool.deinitialize();
	return allocs.deinitialize();
}

void leak_tracker::on_stat(const allocation_stats& stats, void* old_ptr)
{
	internal_on_stat((uintptr_t)stats.pointer, stats, (uintptr_t)old_ptr);
}

void leak_tracker::on_stat_ordinal(const allocation_stats& stats, uint32_t old_ordinal)
{
	internal_on_stat((uintptr_t)stats.ordinal, stats, (uintptr_t)old_ordinal);
}

void leak_tracker::internal_on_stat(uintptr_t new_ptr, const allocation_stats& stats, uintptr_t old_ptr)
{
	concurrent_hash_map::value_type idx = concurrent_hash_map::nullidx;

	switch (stats.operation)
	{
		case memory_operation::reallocate:
			idx = allocs.remove(leak_tracker_hash(old_ptr));
			if (idx == concurrent_hash_map::nullidx)
				oTRACE("[leak_tracker] failed to find old_ptr for realloc");
			// pass thru to allocate

		case memory_operation::allocate:
		{
			if (idx == concurrent_hash_map::nullidx)
				idx = pool.allocate_index();

			if (idx != concurrent_hash_map::nullidx)
			{
				entry* e = pool.typed_pointer(idx);
				e->label = stats.label;
				e->size = stats.size;
				e->num_stack_entries = init.capture_callstack ? static_cast<uint8_t>(init.callstack(e->stack, stack_trace_max_depth, stack_trace_offset)) : 0;
				e->tracked = init.thread_local_tracking_enabled();
				e->context = current_context;
				e->id = stats.ordinal;

				idx = allocs.set(leak_tracker_hash(new_ptr), idx);
				if (idx != concurrent_hash_map::nullidx)
				{
					e = pool.typed_pointer(idx);
					oTRACE("[leak_tracker] hash collision:\n  incoming: ordinal=%u size=%u new_ptr=%p old_ptr=%p ptr=%p\n"
						"  outgoing: ordinal=%u size=%u"
						, stats.ordinal, uint32_t(stats.size), new_ptr, old_ptr, stats.pointer, e->id, uint32_t(e->size));
				}
			}

			break;
		}

		case memory_operation::deallocate:
		{
			idx = allocs.remove(leak_tracker_hash(new_ptr));
			if (idx != concurrent_hash_map::nullidx)
				pool.deallocate(idx);
			break;
		}

		default:
			break;
	}
}

size_t leak_tracker::num_outstanding_allocations(bool current_context_only)
{
	size_t n = 0;
	allocs.visit([&](concurrent_hash_map::value_type idx)
	{
		const entry& e = *pool.typed_pointer(idx);
		if (e.tracked && (!current_context_only || e.context == current_context))
			n++;
	});

	return n;
}

size_t leak_tracker::report(bool current_context_only)
{
	xlstring buf;
	sstring memsize;

	release_delay();
	if (cv_status::timeout == delay_latch.wait_for(chrono::milliseconds(init.expected_delay_ms))) // some delayed frees might be in threads that get stomped on (thus no exit code runs) during static deinit, so don't wait forever
		oTRACE("WARNING: a delay on the leak report count was added, but has yet to be released. The timeout has been reached, so this report will include delayed releases that haven't (yet) occurred.");

	// Some 3rd party task systems (TBB) may not have good API for ensuring they
	// are in a steady-state going into a leak report. To limit the number of 
	// false-positives, check again if there are leaks.
	bool RecoveredFromAsyncLeaks = false;
	size_t CheckedNumLeaks = num_outstanding_allocations(current_context_only);
	if (CheckedNumLeaks > 0)
	{
		oTRACE("There are potentially %u leaks(s), sleeping and checking again to eliminate async false positives...", CheckedNumLeaks);
		this_thread::sleep_for(chrono::milliseconds(init.unexpected_delay_ms));
		RecoveredFromAsyncLeaks = true;
	}

	size_t nLeaks = 0;
	bool headerPrinted = false;
	size_t totalLeakBytes = 0;
	allocs.visit([&](concurrent_hash_map::value_type idx)
	{
		const entry& e = *pool.typed_pointer(idx);
		if (e.tracked && (!current_context_only || e.context == current_context))
		{
			if (!headerPrinted)
			{
				mstring Header;
				snprintf(Header, "========== Leak Report%s ==========\n"
					, RecoveredFromAsyncLeaks ? " (recovered from async false positives)" : "");
				init.print(Header);
				headerPrinted = true;
			}
			
			nLeaks++;
			totalLeakBytes += e.size;

			format_bytes(memsize, e.size, 2);

			if (init.use_hex_for_alloc_id)
				snprintf(buf, "{0x%p} \"%s\" %s\n", e.id, e.label ? e.label : "unlabeled", memsize.c_str());
			else
				snprintf(buf, "{%d} \"%s\" %s\n", e.id, e.label ? e.label : "unlabeled", memsize.c_str());

			init.print(buf);

			bool IsStdBind = false;
			for (size_t i = 0; i < e.num_stack_entries; i++)
			{
				bool WasStdBind = IsStdBind;
				init.format(buf, buf.capacity(), e.stack[i], "  ", &IsStdBind);
				if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
					i += std_bind_internal_offset;
				init.print(buf);
			}
		}
	});

	if (nLeaks)
	{
		sstring strTotalLeakBytes;
		mstring Footer;
		format_bytes(strTotalLeakBytes, totalLeakBytes, 2);
		snprintf(Footer, "========== Leak Report: %u Leak(s) %s%s ==========\n"
			, nLeaks
			, strTotalLeakBytes.c_str()
			, RecoveredFromAsyncLeaks ? " (recovered from async false positives)" : "");
		init.print(Footer);
	}

	delay_latch.reset(1);
	return nLeaks;
}

}
