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
// Sometimes it is necessary to call a system function that takes a timeout
// value in a loop. So from interface/user space we'd like the calling function
// to respect the timeout we gave, even if the lower-level code does something
// less elegant. To encapsulate this common pattern, here is a scoped timeout
// object that decrements a timeout value as code within it's scope executes.
// Often usage of this object implies careful application of {} braces, which 
// is still cleaner code than maintaining timer calls.
// NOTE: If your loop may not take much time to execute, consider creating the
// instance of this class outside the loop and calling UpdateTimeout.
#pragma once
#ifndef oScopedPartialTimeout_h
#define oScopedPartialTimeout_h

#include <oBase/assert.h>

class oScopedPartialTimeout
{
public:
	// Pointer to a timeout value to update. The value should be initialized to 
	// the user-specified total timeout initially and then allowed to be updated 
	// in a loop using UpdateTimeout(). If the value of the timeout is 
	// ouro::infinite, then this class doesn't update the value thus allowing the 
	// ouro::infinite value to be propagated.
	oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown);
	~oScopedPartialTimeout();

	// Call this in a loop that is doing other work to decrement the value pointed
	// to by pTimeoutMSCountdown.
	void UpdateTimeout();

protected:
	unsigned int* pTimeoutMSCountdown;
	unsigned int Start; //needs to be in ms or you can have some serious precision issues causing the timer to never advance. i.e. if each iteration of the loop is less than 0.5ms
};

#endif
