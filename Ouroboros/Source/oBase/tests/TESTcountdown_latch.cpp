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
#include <oBase/countdown_latch.h>
#include <oBase/finally.h>
#include <oStd/for.h>
#include <oStd/future.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {
	namespace tests {

static bool test(int _Count)
{
	int latchCount = _Count;
	std::atomic<int> count(0);
	countdown_latch latch(latchCount);
	
	// NOTE: This pattern is only for testing purposes of countdown latch. In 
	// applications, it's better to async() a task that will then do other tasks
	// and wait on the future of that first async() call... such as async()'ing a 
	// parallel_for. Again, this pattern here is to purposefully exacerbate 
	// and test countdown_latch, thus sync'ing on the futures wouldn't do much 
	// good.

	std::vector<oStd::future<void>> Futures;

	for (int i = 0; i < _Count; i++)
	{
		oStd::future<void> f = oStd::async([&,i]
		{
			oStd::this_thread::sleep_for(oStd::chrono::milliseconds(100 * (i + 1))); // stagger the sleeps to simulate doing work that takes a variable amount of time
			count++;
			latch.release();
		});

		Futures.push_back(std::move(f));
	}

	latch.wait();

	oFOR(oStd::future<void>& f, Futures)
		f.wait();

	return count == latchCount;
}

void TESTcountdown_latch()
{
	oCHECK(test(5), "countdown_latch failed to wait properly.");
	oCHECK(test(1), "countdown_latch failed to wait properly.");
}

	} // namespace tests
} // namespace ouro
