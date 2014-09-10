
// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A vectored exception handler that redirects some other handlers to the 
// main exception path and calls to the handler when an exception occurs.
#pragma once
#ifndef oCore_win_exception_handler_h
#define oCore_win_exception_handler_h

#include <oString/path.h>
#include <functional>
#include <string>

namespace ATL { struct CAtlException; }
class _com_error;

namespace ouro {
	namespace windows {
		namespace exception {

namespace type
{	enum value {

	unknown,
	std,
	com,
	atl,

};}

struct cpp_exception
{
	cpp_exception()
		: type(type::unknown)
		, type_name("")
		, what("")
	{ void_exception = nullptr; }

	type::value type;
	const char* type_name;
	std::string what;
	union
	{
		std::exception* std_exception;
		_com_error* com_error;
		ATL::CAtlException* atl_exception;
		void* void_exception;
	};
};

// NOTE: _pStdException may be nullptr if the exception is not derived from 
// std::exception.
typedef std::function<void(const char* _Message
	, const cpp_exception& _CppException
	, uintptr_t _ExceptionContext)> handler;

void set_handler(const handler& _Handler);

// Specify a directory where mini/full dumps will be written. Dump filenames 
// will be generated with a timestamp at the time of exception.
void mini_dump_path(const path& _MiniDumpPath);
const path& mini_dump_path();

void full_dump_path(const path& _FullDumpPath);
const path& full_dump_path();

void post_dump_command(const char* _Command);
const char* post_dump_command();

void prompt_after_dump(bool _Prompt);
bool prompt_after_dump();

// Set the enable state of debug CRT asserts and errors
void enable_dialogs(bool _Enable);

		} // namespace exception
	} // namespace windows
} // namespace ouro

#endif
