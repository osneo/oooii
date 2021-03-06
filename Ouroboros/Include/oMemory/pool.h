// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// O(1) block allocator: uses space inside free blocks to maintain freelist.

#pragma once
#include <cstdint>

namespace ouro
{

class pool
{
public:
	typedef uint32_t index_type;
	typedef uint32_t size_type;

	// if at capacity allocate() will return this value
	static const index_type nullidx = 0xffffffff;

	// largest index issued by this pool, also serving as a static max_capacity()
	static const size_type max_index = nullidx - 1;

	// the maximum number of items a pool can hold
	static size_type max_capacity() { return max_index; }

	// returns the size the memory param must be in init methods below
	static size_type calc_size(size_type block_size, size_type capacity);

	// ctor creates as empty
	pool();

	// ctor moves an existing pool into this one
	pool(pool&& that);

	// ctor creates as a valid pool using external memory
	pool(void* memory, size_type block_size, size_type capacity);

	// dtor
	~pool();

	// calls deinitialize on this, moves that's memory under the same config
	pool& operator=(pool&& that);

	// use calc_size() to determine memory size
	void initialize(void* memory, size_type block_size, size_type capacity);

	// deinitializes the pool and returns the memory passed to initialize()
	void* deinitialize();

	// returns the size each allocated block will be
	inline size_type block_size() const { return stride; }

	// returns the number of items available
	inline size_type size_free() const { return nfree; }

	// returns the number of allocated elements
	inline size_type size() const { return capacity() - size_free(); }

	// returns true of there are no outstanding allocations
	inline bool full() const { return capacity() == size_free(); }

	// returns the max number of items that can be allocated from this pool
	inline size_type capacity() const { return nblocks; }

	// returns true if all items have been allocated
	inline bool empty() const { return head == nullidx; }

	// allocate/deallocate an index into the pool. If empty out of resources 
	// nullidx will be returned.
	index_type allocate_index();
	void deallocate(index_type index);

	// Wrappers for treating the block as a pointer to block_size memory.
	inline void* allocate() { index_type i = allocate_index(); return pointer(i); }
	inline void deallocate(void* pointer) { deallocate(index(pointer)); }

	// convert between allocated index and pointer values
	void* pointer(index_type index) const;
	index_type index(void* ptr) const;

	// simple range check that returns true if this index/pointer could have been 
	// allocated from this pool
	bool owns(index_type index) const { return index < nblocks; }
	bool owns(void* ptr) const { return owns(index(ptr)); }

private:
	uint8_t* blocks;
	size_type stride;
	size_type nblocks;
	size_type nfree;
	uint32_t head;

	pool(const pool&); /* = delete; */
	const pool& operator=(const pool&); /* = delete; */
};

}
