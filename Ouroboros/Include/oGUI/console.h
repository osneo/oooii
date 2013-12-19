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
// Interface for working with a console in a GUI environment
#pragma once
#ifndef oGUI_console_h
#define oGUI_console_h

#include <oBase/color.h>
#include <oGUI/oGUI.h>
#include <cstdarg>
#include <functional>

namespace ouro { 
	namespace console {

static const int use_default = 0x8000;

enum signal
{
  ctrl_c,
  ctrl_break,
  close,
  logoff,
  shutdown,
};

struct info
{
	info()
		: window_position(use_default, use_default)
		, window_size(use_default, use_default)
		, buffer_size(use_default, use_default)
		, foreground(0)
		, background(0)
		, show(true)
	{}

  int2 window_position;
  int2 window_size;
  int2 buffer_size;
  color foreground;
  color background;
	bool show;
};
  
ouro::window_handle native_handle();
  
info get_info();
void set_info(const info& _Info);
  
inline int2 position() { info i = get_info(); return i.window_position; }
inline void position(const int2& _Position) { info i = get_info(); i.window_position = _Position; set_info(i); }
  
inline int2 size() { info i = get_info(); return i.window_size; }
inline void size(const int2& _Size) { info i = get_info(); i.window_size = _Size; set_info(i); }
        
void set_title(const char* _Title);
char* get_title(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> char* get_title(char (&_StrDestination)[size]) { return get_title(_StrDestination, size); }
template<size_t capacity> char* get_title(fixed_string<char, capacity>& _StrDestination) { return get_title(_StrDestination, _StrDestination.capacity()); }

// Set to the empty string to disable logging
void set_log(const path& _Path);
path get_log();

void icon(ouro::icon_handle _hIcon);
ouro::icon_handle icon();
  
void focus(bool _Focus);
bool has_focus();
  
int2 size_pixels();
int2 size_characters();
  
void cursor_position(const int2& _Position);
int2 cursor_position();
  
// The handler should return true to short-circuit any default behavior, or 
// false to allow default behavior to continue.
void set_handler(signal _Signal, const std::function<bool()>& _Handler);
  
void clear();
  
int vfprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, va_list _Args);
inline int vfprintf(FILE* _pStream, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(_pStream, color(0), color(0), _Format, args); va_end(args); return n; }
inline int fprintf(FILE* _pStream, color _Foreground, color _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(_pStream, _Foreground, _Background, _Format, args); va_end(args); return n; }
inline int fprintf(FILE* _pStream, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(_pStream, _Format, args); va_end(args); return n; }
inline int printf(color _Foreground, color _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(stdout, _Foreground, _Background, _Format, args); va_end(args); return n; }
inline int printf(const char* _Format, ...) { va_list args; va_start(args, _Format); int n = vfprintf(stdout, _Format, args); va_end(args); return n; }

	} // namespace console
} // namespace ouro

#endif
