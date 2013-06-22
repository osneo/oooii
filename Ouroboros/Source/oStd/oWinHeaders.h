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
#pragma once
#ifndef oWinHeaders_h
#define oWinHeaders_h

#if !defined(_WIN32) && !defined(_WIN64)
	#error Including a Windows header for a non-Windows target
#endif

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <oWinHeaders.h> to limit the proliferation of macros and platform interfaces throughout the code.")
#endif

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#define NOATOM
#define NOCOMM
#define NOCRYPT
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDICAPMASKS
#define NOHELP
#define NOIMAGE
#define NOKANJI
#define NOKERNEL
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOOPENFILE
#define NOPROFILER
#define NOPROXYSTUB
#define NORASTEROPS
#define NORPC
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTAPE
#define WIN32_LEAN_AND_MEAN

#ifdef interface
	#undef interface
	#undef INTERFACE_DEFINED
#endif

#undef _M_CEE_PURE

#include <windows.h>

// Define a low-level assertion here separate from oAssert.h because:
// A. it protects against any complexities in user implementations of oASSERT
// B. When oStd code is replaced compiler-standard libs we won't have control 
//    anyway.

#ifdef _DEBUG
	#include <crtdbg.h>
	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "Ouroboros Debug Library", #expr "\n\n" msg, ## __VA_ARGS__)) __debugbreak(); }
	#define oCRTTRACE(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "Ouroboros Debug Library", msg "\n", ## __VA_ARGS__); } while(false)
#else
	#define oCRTASSERT(expr, msg, ...) __noop
	#define oCRTTRACE(msg, ...) __noop
#endif

#endif