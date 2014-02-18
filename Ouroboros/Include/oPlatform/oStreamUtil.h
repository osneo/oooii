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
// Functions built on top of the oStream API to simplify the act of loading a 
// buffer and parsing that buffer as a specific known format. These are simple-
// use utilities, no special streaming or windowed loading is done: the whole
// file must fit in memory plus the structured output must be finished before 
// the source buffer is freed.
#pragma once
#ifndef oStreamUtil_h
#define oStreamUtil_h

#include <vector>
#include <oBase/csv.h>
#include <oBase/ini.h>
#include <oBase/xml.h>

// Prefer using oBufferLoad to oStreamLoad - it's more RAII and encapsulates the
// allocation. But for some platform APIs, oStreamLoad needs to be exposed.
// oStreamLoad will load the specified URI references and allocate a buffer to 
// fit that file, then read in the file and assign the size to _pOutSize. If 
// _AsString is true, a null-terminator will be added. Both an allocation and
// deallocation function must be specified. Deallocation is only done if there 
// is a read error.
bool oStreamLoad(void** _ppOutBuffer, size_t* _pOutSize, const std::function<void*(size_t _NumBytes)>& _Allocate, const std::function<void(void* _Pointer)>& _Deallocate, const char* _URIReference, bool _AsString);
template<typename T> bool oStreamLoad(T** _ppOutBuffer, size_t* _pOutSize, const std::function<void*(size_t _NumBytes)>& _Allocate, const std::function<void(void* _Pointer)>& _Deallocate, const char* _URIReference, bool _AsString = false) { return oStreamLoad((void**)_ppOutBuffer, _pOutSize, _Allocate, _Deallocate, _URIReference, _AsString); }

bool oStreamLoadPartial(void* _pBuffer, size_t _SizeofBuffer, const char* _URIReference);
template<typename T> bool oStreamLoadPartial(T* _pBuffer, const char* _URIReference) { return oStreamLoadPartial()}

bool oBufferLoad(const char* _URIReference, interface oBuffer** _ppBuffer, bool _AsString = false);

std::shared_ptr<ouro::csv> oCSVLoad(const char* _URIReference);
std::shared_ptr<ouro::ini> oINILoad(const char* _URIReference);
std::shared_ptr<ouro::xml> oXMLLoad(const char* _URIReference);

#endif
