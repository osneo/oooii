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
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oError.h>
#include <oBase/finally.h>
#include "oBasisTestCommon.h"
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace ouro;
using namespace std;

static void SetLocation(size_t _Index, size_t _Start, int* _Array)
{
	int startValue = _Array[_Start - 1];
	_Array[_Index] = startValue + static_cast<int>(_Index + 1 - _Start);
}

static void FillArray(int* _Array, size_t _Start, size_t _End, std::thread::id* _pExecutionThreadID, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID == thread::id()) // this command should execute before any others.
		*_pExecutionThreadID = this_thread::get_id();

	if (*_pExecutionThreadID != this_thread::get_id())
		*_pWrongThreadError = true;

	oConcurrency::parallel_for(_Start, _End, std::bind(&SetLocation, std::placeholders::_1, _Start, _Array));
}

static void CheckTest(int* _Array, size_t _Size, bool* _pResult, thread::id* _pExecutionThreadID, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID != this_thread::get_id())
		*_pWrongThreadError = true;

	*_pResult = false;
	for (size_t i = 0; i < _Size; i++)
		if (_Array[i] != static_cast<int>(i))
			return;
	*_pResult = true;
}

static void NotifyAll(mutex& _Mutex, condition_variable& _ConditionVariable, thread::id* _pExecutionThreadID, bool* _pNotified, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID != this_thread::get_id())
		*_pWrongThreadError = true;

	{
		lock_guard<mutex> Lock(_Mutex);
		*_pNotified = true;
	}

	_ConditionVariable.notify_all();
}

bool oBasisTest_oDispatchQueuePrivate()
{
	intrusive_ptr<threadsafe oDispatchQueuePrivate> q;
	oTESTB(oDispatchQueueCreatePrivate("TESTDispatchQueuePrivate", 100, &q), "Failed to create private dispatch queue");
	finally JoinQueue([&] { q->Join(); });

	static const size_t TestSize = 4096;
	int TestArray[TestSize];

	for(int i = 0; i < 2; ++i)
	{
		bool bResult = false;

		condition_variable Finished;
		mutex FinishedMutex;

		memset(TestArray, -1, TestSize * sizeof(int));
		TestArray[0] = 0;

		thread::id ExecutionThreadID;
		bool WrongThread = false;

		q->Dispatch(&FillArray, TestArray, 1, 1024, &ExecutionThreadID, &WrongThread);
		q->Dispatch(&FillArray, TestArray, 1024, 2048, &ExecutionThreadID, &WrongThread);
		q->Dispatch(&FillArray, TestArray, 2048, TestSize, &ExecutionThreadID, &WrongThread);
		q->Dispatch(&CheckTest, TestArray, TestSize, &bResult, &ExecutionThreadID, &WrongThread);
		
		bool Notified = false;
		q->Dispatch(&NotifyAll, std::ref(FinishedMutex), std::ref(Finished), &ExecutionThreadID, &Notified, &WrongThread);

		// @tony: there's a race here because the notify can happen before we settle into the wait, so the notify never comes through.

		unique_lock<mutex> FinishedLock(FinishedMutex);
		while (!Notified)
			Finished.wait(FinishedLock);

		oTESTB(bResult, "oDispatchQueuePrivate failed to preserve order!");
		oTESTB(!WrongThread, "oDispatchQueuePrivate command was not executing on the correct thread.");
	}

	oErrorSetLast(0);
	return true;
}
