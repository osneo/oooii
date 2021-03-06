// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
