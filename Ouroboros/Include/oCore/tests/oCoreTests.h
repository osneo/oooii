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
// Declarations of oCore unit tests. These throw on failure.
#pragma once
#ifndef oCoreTests_h
#define oCoreTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTadapter(test_services& _Services);
		void TESTcamera(test_services& _Services);
		void TESTcpu(test_services& _Services);
		void TESTdebugger(test_services& _Services);
		void TESTfilesystem(test_services& _Services);
		void TESTfilesystem_monitor();
		void TESTprocess_heap();
		#if defined(_WIN32) || defined(_WIN64)
			void TESTwin_crt_leak_tracker(test_services& _Services);
			void TESTwin_registry();
		#endif

	} // namespace tests
} // namespace ouro

#endif
