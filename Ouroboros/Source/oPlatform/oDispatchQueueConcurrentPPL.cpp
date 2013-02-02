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
#include "oDispatchQueueConcurrentPPL.h"
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oJoinable.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oRefCount.h>
#include <ppl.h>

struct oDispatchQueueConcurrentPPLImpl : public oDispatchQueueConcurrent
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oDispatchQueueConcurrentPPLImpl(const char* _DebugName, size_t _InitialTaskCapacity, bool* _pSuccess)
		: DebugName(_DebugName)
	{
		*_pSuccess = true;
	}

	bool Dispatch(oTASK _Command) threadsafe override
	{
		if (J.BeginDispatch())
		{
			oInherentlyThreadsafe()->TaskGroup.run(std::move(_Command));
			J.EndDispatch();
			return true;
		}
		
		return false;
	}

	void Join() threadsafe override { J.Join(); oInherentlyThreadsafe()->TaskGroup.wait(); }
	void Flush() threadsafe override { J.BeginFlush(); oInherentlyThreadsafe()->TaskGroup.wait(); J.EndFlush(); }
	bool Joinable() const threadsafe override { return J.Joinable(); }
	const char* GetDebugName() const threadsafe override { return *DebugName; }

	oInitOnce<oStringS> DebugName;
	Concurrency::task_group TaskGroup;
	oJoinable J;
	oRefCount RefCount;
};

bool oDispatchQueueCreateConcurrentPPL(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
{
	bool success = false;
	oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentPPLImpl(_DebugName, _InitialTaskCapacity, &success));
	return success;
}