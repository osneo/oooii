/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Utilities for working with the default heap used by the platform's C runtime.
// Often the default malloc/free API is supported by robust introspection and 
// debugging facilities, and if not they should be added! So here is a cross-
// platform API for interacting with memory based on the robust Microsoft 
// crtdbg feature set.
#pragma once
#ifndef oCore_win_crt_heap_h
#define oCore_win_crt_heap_h

#include <stddef.h>

namespace ouro {
	namespace windows {
		namespace crt_heap {

// Returns true if the specified pointer is allocated the MSVCRT malloc heap, 
// false if nullptr or not a pointer that can be passed to free (i.e. an offset
// pointer)
bool is_valid(void* _Pointer);

// Returns a pointer as it is stored at the specified block.
void* get_pointer(struct _CrtMemBlockHeader* _pMemBlockHeader);

// Given a pointer, retrieve the 'next' pointer as it is stored inside CRT.
// For example, starting with the pointer from a _CrtMemState you can traverse
// all allocations since that state.
void* next_pointer(void* _Pointer);

// Returns the user size of the allocation. This assumes 
size_t size(void* _Pointer);

// Returns true if the specified pointer is free-but-still-allocated. This is 
// a special debug mode that doesn't make freed memory available for future
// allocation and thus tags certain blocks in this manner.
// This assumes the specified pointer is valid.
bool is_free(void* _Pointer);

// Returns true if this is a typical allocation
// This assumes the specified pointer is valid.
bool is_normal(void* _Pointer);

// Returns true if this is an allocation internal to the CRT itself
// This assumes the specified pointer is valid.
bool is_crt(void* _Pointer);

// Returns true if this is an allocation that has been marked as ignore-by-
// tracking so while this allocation exists, it would not be reported as a leak.
// This assumes the specified pointer is valid.
bool is_ignore(void* _Pointer);

// Returns true if this is an allocation explicitly flagged as a client 
// allocation.
// This assumes the specified pointer is valid.
bool is_client(void* _Pointer);

// Returns the name of the source code where the specified pointer was 
// allocated. This assumes the specified pointer is valid.
const char* allocation_filename(void* _Pointer);

// Returns the line in the source code where the specified pointer was 
// allocated. This assumes the specified pointer is valid.
unsigned int allocation_line(void* _Pointer);

// Returns a unqiue identifier for the specified allocation. This can be the
// pointer itself, but that is not always the case. This assumes the specifed 
// pointer is valid.
unsigned int allocation_id(void* _Pointer);

// Causes execution to break into the debugger when the specified allocation
// occurs.
void break_on_allocation(uintptr_t _AllocationID);

// If true, a leak report will be printed out at program exit. This is false by
// default to allow more robust methods to be used.
void enable_at_exit_leak_report(bool _Enable = true);

// If true, all allocations are tracked. If false, memory is allocated as an 
// IGNORE block and anyone testing with oCRTHeapIsIgnore can use that to 
// determine whether to track that memory or not. The default tracker will not
// track the entries when this is disabled. Use this to blacklist false-positive
// memory allocations such as static-time 3rd party middleware allocations such
// as those done by TBB.
bool enable_memory_tracking(bool _Enable = true);

		} // namespace crt_heap
	} // namespace windows
} // namespace ouro

#endif
