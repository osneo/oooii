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
// A small class that spawns a thread to monitor CPU usage by the calling 
// process.
#pragma once
#ifndef oProcessStatsMonitor_h
#define oProcessStatsMonitor_h

#include <oBasis/oMath.h>
#include <oStd/oStdThread.h>
#include <oConcurrency/backoff.h>
#include <oConcurrency/oConcurrency.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oProcess.h>

struct oPROCESS_CPU_STATS
{
	float AverageUsage;
	float LowUsage;
	float HighUsage;
};

class oProcessStatsMonitor
{
public:
	inline oProcessStatsMonitor(unsigned int _ProcessID = oInvalid, oStd::chrono::milliseconds _PollRate = oStd::chrono::milliseconds(1000))
		: PID(_ProcessID == oInvalid ? oProcessGetCurrentID() : _ProcessID)
		, Done(true)
		, PollRate(_PollRate)
	{
		Reset();
		Thread = std::move(oStd::thread(&oProcessStatsMonitor::Proc, this));
		oConcurrency::backoff bo;
		while (Done)
			bo.pause();
	}

	inline ~oProcessStatsMonitor()
	{
		Join();
	}

	inline void GetStats(oPROCESS_CPU_STATS* _pStats) const
	{
		oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(StatsMutex);
		*_pStats = Stats;
	}

	inline void Reset()
	{
		Stats.AverageUsage = 0.0f;
		Stats.LowUsage = 100.0f;
		Stats.HighUsage = 0.0f;
		MA = oMovingAverage<double>();
		PreviousSystemTime = 0;
		PreviousProcessTime = 0;
	}

private:
	inline void Proc()
	{
		oConcurrency::begin_thread("oPrcessStatsMonitor");
		Done = false;

		oProcessCalculateCPUUsage(PID, &PreviousSystemTime, &PreviousProcessTime);
		do
		{
			oStd::this_thread::sleep_for(PollRate);
			double usage = oProcessCalculateCPUUsage(PID, &PreviousSystemTime, &PreviousProcessTime);
			float usagef = static_cast<float>(usage);
			float avg = static_cast<float>(MA.Calc(usage));
			oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(StatsMutex);
			Stats.AverageUsage = avg;
			Stats.LowUsage = __min(Stats.LowUsage, usagef);
			Stats.HighUsage = __max(Stats.HighUsage, usagef);

		} while (!Done);

		oConcurrency::end_thread();
	}

	inline void Join()
	{
		Done = true;
		Thread.join();
	}

	oConcurrency::shared_mutex StatsMutex;
	oPROCESS_CPU_STATS Stats;
	unsigned long long PreviousSystemTime;
	unsigned long long PreviousProcessTime;
	oMovingAverage<double> MA;
	oStd::chrono::milliseconds PollRate;
	unsigned int PID;
	bool Done;
	oStd::thread Thread;
};

#endif
