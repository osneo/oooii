/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include "oAllocatorTLSF.h"
#include <oBasis/oByte.h>
#include <oBasis/oString.h>
#include "tlsf.h"

const oGUID& oGetGUID(threadsafe const oAllocatorTLSF* threadsafe const *)
{
	// {5CFA8784-E09D-40e3-8C7A-C4809577F02F}
	static const oGUID oIIDAllocatorTLSF = { 0x5cfa8784, 0xe09d, 0x40e3, { 0x8c, 0x7a, 0xc4, 0x80, 0x95, 0x77, 0xf0, 0x2f } };
	return oIIDAllocatorTLSF;
}

AllocatorTLSF_Impl::AllocatorTLSF_Impl(const char* _DebugName, const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, DebugName(_DebugName)
{
	*_pSuccess = false;
	AllocatorTLSF_Impl::Reset();
	if (!AllocatorTLSF_Impl::IsValid())
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to construct TLSF allocator");
		return;
	}
	*_pSuccess = true;
}

void TraceAllocated(void* ptr, size_t size, int used, void* user)
{
	if (used)
	{
		oStringS mem;
		oTRACE("TLSF LEAK %s: 0x%p %s", (const char*)user, ptr, oFormatMemorySize(mem, size, 2));
	}
}

AllocatorTLSF_Impl::~AllocatorTLSF_Impl()
{
	if (Stats.NumAllocations != 0)
		tlsf_walk_heap(hPool, TraceAllocated, (void*)DebugName->c_str());
	oASSERT(Stats.NumAllocations == 0, "Allocator %s being destroyed with %u allocations still unfreed! This may leave dangling pointers. See trace for addresses.", DebugName, Stats.NumAllocations);
	oASSERT(AllocatorTLSF_Impl::IsValid(), "TLSF Heap is corrupt");
	tlsf_destroy(hPool);
}

bool oAllocatorCreateTLSF(const char* _DebugName, const oAllocator::DESC& _Desc, oAllocator** _ppAllocator)
{
	if (!_ppAllocator || !_Desc.pArena || !_Desc.ArenaSize)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	bool success = false;
	oCONSTRUCT_PLACEMENT(_ppAllocator, _Desc.pArena, AllocatorTLSF_Impl(_DebugName, _Desc, &success));
	return true;
}

void AllocatorTLSF_Impl::GetDesc(DESC* _pDesc)
{
	*_pDesc = Desc;
}

void AllocatorTLSF_Impl::GetStats(STATS* _pStats)
{
	*_pStats = Stats;
}

const char* AllocatorTLSF_Impl::GetDebugName() const threadsafe
{
	return DebugName->c_str();
}

const char* AllocatorTLSF_Impl::GetType() const threadsafe
{
	return "TLSF";
}

bool AllocatorTLSF_Impl::IsValid()
{
	return hPool && !tlsf_check_heap(hPool);
}

void* AllocatorTLSF_Impl::Allocate(size_t _NumBytes, size_t _Alignment)
{
	void* p = tlsf_memalign(hPool, _Alignment, __max(_NumBytes, 1));
	if (p)
	{
		size_t blockSize = tlsf_block_size(p);
		Stats.NumAllocations++;
		Stats.BytesAllocated += blockSize;
		Stats.BytesFree	-= blockSize;
		Stats.PeakBytesAllocated = __max(Stats.PeakBytesAllocated, Stats.BytesAllocated);
	}

	return p;
}

void* AllocatorTLSF_Impl::Reallocate(void* _Pointer, size_t _NumBytes)
{
	size_t oldBlockSize = _Pointer ? tlsf_block_size(_Pointer) : 0;
	void* p = tlsf_realloc(hPool, _Pointer, _NumBytes);
	if (p)
	{
		size_t blockSizeDiff = tlsf_block_size(p) - oldBlockSize;
		Stats.BytesAllocated += blockSizeDiff;
		Stats.BytesFree	-= blockSizeDiff;
		Stats.PeakBytesAllocated = __max(Stats.PeakBytesAllocated, Stats.BytesAllocated);
	}

	return 0;
}

void AllocatorTLSF_Impl::Deallocate(void* _Pointer)
{
	if (_Pointer)
	{
		size_t blockSize = tlsf_block_size(_Pointer);
		tlsf_free(hPool, _Pointer);
		Stats.NumAllocations--;
		Stats.BytesAllocated -= blockSize;
		Stats.BytesFree += blockSize;
	}
}

size_t AllocatorTLSF_Impl::GetBlockSize(void* _Pointer)
{
	return tlsf_block_size(_Pointer);
}

void AllocatorTLSF_Impl::Reset()
{
	memset(&Stats, 0, sizeof(Stats));
	void* pRealArenaStart = oByteAlign(oByteAdd(Desc.pArena, sizeof(*this)), oDEFAULT_MEMORY_ALIGNMENT);
	size_t realArenaSize = Desc.ArenaSize - std::distance((char*)Desc.pArena, (char*)pRealArenaStart);
	hPool = tlsf_create(pRealArenaStart, realArenaSize);
	Stats.NumAllocations = 0;
	Stats.BytesAllocated = 0;
	Stats.PeakBytesAllocated = 0;
	Stats.BytesFree = Desc.ArenaSize - tlsf_overhead();
}

static void TLSFWalker(void* ptr, size_t size, int used, void* user)
{
	oFUNCTION<void(void* _Pointer, size_t _Size, bool _IsUserAllocation)>& Walker = *(oFUNCTION<void(void* _Pointer, size_t _Size, bool _Used)>*)user;
	Walker(ptr, size, !!used);
}

void AllocatorTLSF_Impl::WalkHeap(oFUNCTION<void(void* _Pointer, size_t _Size, bool _Used)> _HeapWalker)
{
	if (_HeapWalker)
		tlsf_walk_heap(hPool, TLSFWalker, &_HeapWalker);
}