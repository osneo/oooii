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
// Read / Write info to the registry
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724897%28v=vs.85%29.aspx
#pragma once
#ifndef oCore_win_registry_h
#define oCore_win_registry_h

#include <oBase/fixed_string.h>

namespace ouro {
	namespace windows {
		namespace registry {

enum hkey
{
	classes_root,
	current_user,
	local_machine,
	users,
};

void delete_value(hkey _hKey, const char* _KeyPath, const char* _ValueName);
void delete_key(hkey _hKey, const char*_KeyPath, bool _Recursive = true);

// Sets the specified key's value to _pValue as a string. If the key path does 
// not exist, it is created. If _pValueName is null or "", then the (default) 
// value key.
void set(hkey _hKey, const char* _KeyPath, const char* _ValueName, const char* _Value);

// Fills _StrDestination with the specified Key's path. Returns _StrDestination
// or nullptr if the specified key does not exist.
char* get(char* _StrDestination, size_t _SizeofStrDestination, hkey _hKey, const char* _KeyPath, const char* _ValueName);
template<size_t size> char* get(char (&_StrDestination)[size], hkey _hKey, const char* _KeyPath, const char* _ValueName) { return get(_StrDestination, size, _hKey, _KeyPath, _ValueName); }
template<size_t capacity> char* get(ouro::fixed_string<char, capacity>& _StrDestination, hkey _hKey, const char* _KeyPath, const char* _ValueName) { return get(_StrDestination, _StrDestination.capacity(), _hKey, _KeyPath, _ValueName); }

template<typename T> bool get(T* _pTypedValue, hkey _hKey, const char* _KeyPath, const char* _ValueName)
{
	sstring buf;
	if (!get(buf, _Root, _KeyPath, _ValueName))
		return false;
	return from_string(_pTypedValue, buf);
}

		} // namespace registry
	} // namespace windows
} // namespace ouro

#endif
