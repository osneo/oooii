/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
#include <oConcurrency/oConcurrency.h>

#include <oBasis/oDispatchQueueConcurrentT.h>
#include <oConcurrency/task_group_threadpool.h>

#undef interface
#undef INTERFACE_DEFINED

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>

using namespace tbb;
typedef task_group task_group_t;

typedef oConcurrency::task_group_threadpool<task_group> threadpool_t;

const oGUID& oGetGUID(threadsafe const oDispatchQueueConcurrentT<threadpool_t>* threadsafe const*)
{
	// {d912957c-a621-4960-833f-55572a1a4abb}
	static const oGUID IID_oDispatchQueueConcurrentT = { 0xd912957c, 0xa621, 0x4960, { 0x83, 0x3f, 0x55, 0x57, 0x2a, 0x1a, 0x4a, 0xbb } };
	return IID_oDispatchQueueConcurrentT;
}

class task_group_impl : public ouro::task_group
{
	task_group_t g;
public:
	#if oHAS_oCONCURRENCY
		task_group_impl() : oConcurrency::detail::task_group<oScheduler::allocator_type>(oScheduler::Singleton()->Threadpool) {}
	#endif
	void run(const std::function<void()>& _Task) override { g.run(_Task); }
	void wait() override { g.wait(); }
	~task_group_impl() { wait(); }
};

bool oDispatchQueueCreateConcurrent(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueConcurrent** _ppQueue)
{
	bool success = false;
	oCONSTRUCT(_ppQueue, oDispatchQueueConcurrentT<threadpool_t>(_DebugName, &success));
	return success;
}
