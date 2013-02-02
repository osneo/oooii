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
// Approximation of the upcoming C++11 std::thread interface. There has been no
// attention paid to proper exception behavior.
#pragma once
#ifndef oStdThread_h
#define oStdThread_h

#include <oBasis/oCallable.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oStdChrono.h>
#include <functional>

namespace oStd {

	class thread
	{
	public:
		class id;
		typedef void* native_handle_type;

		// Ctors either start the thread in an invalid (nonjoinable) state, or have
		// their thread proc specified.
		thread();
		oDEFINE_CALLABLE_CTOR_WRAPPERS(explicit, thread, initialize);

		// !!! NOTE !!!	All threads must be join()'ed before allowing the thread 
		// to exit or, according to the C++11 standard, std::terminate will be 
		// called thus exiting the application.
		~thread();

		#ifdef oHAS_MOVE_CTOR
			thread(thread&& _That) { *this = std::move(_That); }
			thread& operator=(thread&& _That) { move(_That); return *this; }
		#else
			void move_ctor(thread& _That) { move(*this, _That); }
			thread& move_operator_eq(thread& _That) { move(*this, _That); return *this; }
		#endif

		// Uses move operator to swap this and the specified thread
		void swap(thread& _That);

		// Block the calling thread until this thread's callable function exits
		// (basically a wait() call)
		void join();

		// Set this thread's ID to identity and no longer allow control of the 
		// thread from this interface
		void detach();

		// Returns true if join can still be called (i.e. if the thread isn't 
		// detached and hasn't yet exited)
		bool joinable() const;

		id get_id() const;
		native_handle_type native_handle();
		static unsigned int hardware_concurrency();

		void* hThread;
		void initialize(oCALLABLE _ThreadProc);

		#ifdef oHAS_MOVE_CTOR
			thread(const thread&)/* = delete*/;
			const thread& operator=(const thread&)/* = delete*/;
		#endif

		void move(thread& _That);
	};

	class thread::id
	{	unsigned int ID;
	public:
		id();
	};

	namespace this_thread
	{
		thread::id get_id();
		void yield();
		void __sleep_for(unsigned int _Milliseconds);

		template<typename Rep, typename Period> void sleep_for(const chrono::duration<Rep, Period>& _Duration)
		{
			chrono::milliseconds s = chrono::duration_cast<chrono::milliseconds>(_Duration);
			__sleep_for(static_cast<unsigned int>(s.count()));
		}

	} // namespace this_thread

} // namespace oStd

namespace std
{
	void swap(oStd::thread& _This, oStd::thread& _That);
	bool operator==(oStd::thread::id x, oStd::thread::id y);
	bool operator!=(oStd::thread::id x, oStd::thread::id y);
	bool operator<(oStd::thread::id x, oStd::thread::id y);
	bool operator<=(oStd::thread::id x, oStd::thread::id y);
	bool operator>(oStd::thread::id x, oStd::thread::id y);
	bool operator>=(oStd::thread::id x, oStd::thread::id y);

} // namespace std

inline bool operator==(oStd::thread::id x, oStd::thread::id y) { return std::operator==(x, y); }
inline bool operator!=(oStd::thread::id x, oStd::thread::id y) { return std::operator!=(x, y); }
inline bool operator<(oStd::thread::id x, oStd::thread::id y) { return std::operator<(x, y); }
inline bool operator<=(oStd::thread::id x, oStd::thread::id y) { return std::operator<=(x, y); }
inline bool operator>(oStd::thread::id x, oStd::thread::id y) { return std::operator>(x, y); }
inline bool operator>=(oStd::thread::id x, oStd::thread::id y) { return std::operator>=(x, y); }

#endif