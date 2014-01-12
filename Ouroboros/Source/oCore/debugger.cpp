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
#include <oCore/debugger.h>
#include <oCore/filesystem.h>
#include <oCore/module.h>
#include <oCore/process_heap.h>
#include <oCore/system.h>
#include <oCore/windows/win_util.h>
#include <oCore/windows/win_exception_handler.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>

using namespace std;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

namespace ouro {
	namespace debugger {

void thread_name(const char* _Name, thread::id _ID)
{
	if (_Name && *_Name && IsDebuggerPresent())
	{
		// http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		Sleep(10);
		THREADNAME_INFO i;
		i.dwType = 0x1000;
		i.szName = _Name;
		i.dwThreadID = _ID == std::thread::id() ? GetCurrentThreadId() : asdword(_ID);
		i.dwFlags = 0;
		const static DWORD MS_VC_EXCEPTION = 0x406D1388;
		__try { RaiseException(MS_VC_EXCEPTION, 0, sizeof(i)/sizeof(ULONG_PTR), (ULONG_PTR*)&i); }
		__except(EXCEPTION_EXECUTE_HANDLER) {}
	}
}

void print(const char* _String)
{
	OutputDebugStringA(_String);
}

void break_on_alloc(uintptr_t _AllocationID)
{
	_CrtSetBreakAlloc((long)_AllocationID);
}

// Old StackWalk64. CaptureStackBackTrace is simpler and quicker.
#if 0
/** $(Citation)
	<citation>
		<usage type="Adaptation" />
		<author name="Jochen Kalmback" />
		<description url="http://jpassing.wordpress.com/category/win32/" />
		<license type="BSD" url="http://www.opensource.org/licenses/bsd-license.php" />
	</citation>
	<citation>
		<usage type="Adaptation" />
		<author name="Geoff Evans" />
		<author name="Paul Haile" />
		<description url="http://nocturnal.insomniacgames.com/index.php/Main_Page" />
		<license type="Insomniac Open License (IOL)" url="http://nocturnal.insomniacgames.com/index.php/Insomniac_Open_License" />
	</citation>
*/

// Several of the API assume use of CHAR, not TCHAR
_STATIC_ASSERT(sizeof(CHAR) == sizeof(char));

static void init_context(CONTEXT& c)
{
	memset(&c, 0, sizeof(c));
	RtlCaptureContext(&c);
}

static void init_stack_frame(STACKFRAME64& s, const CONTEXT& c)
{
  memset(&s, 0, sizeof(s));
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Mode = AddrModeFlat;

	#ifdef _M_IX86
	  s.AddrPC.Offset = c.Eip;
	  s.AddrFrame.Offset = c.Ebp;
	  s.AddrStack.Offset = c.Esp;
	#elif _M_X64
	  s.AddrPC.Offset = c.Rip;
	  s.AddrFrame.Offset = c.Rsp;
	  s.AddrStack.Offset = c.Rsp;
	#elif _M_IA64
	  s.AddrPC.Offset = c.StIIP;
	  s.AddrFrame.Offset = c.IntSp;
	  s.AddrStack.Offset = c.IntSp;
	  s.AddrBStore.Offset = c.RsBSP;
	  s.AddrBStore.Mode = AddrModeFlat;
	#else
		#error "Platform not supported!"
	#endif
}

static DWORD image_type()
{
	#ifdef _M_IX86
		return IMAGE_FILE_MACHINE_I386;
	#elif _M_X64
		return IMAGE_FILE_MACHINE_AMD64;
	#elif _M_IA64
		return IMAGE_FILE_MACHINE_IA64;
	#else
		#error Unsupported platform
	#endif
}

size_t callstack(symbol* _pSymbols, size_t _NumSymbols, size_t _Offset)
{

	CONTEXT c;
	init_context(c);

	STACKFRAME64 s;
	init_stack_frame(s, c);

	size_t n = 0;
	while (n < _NumSymbols)
	{
		if (!StackWalk64(image_type(), GetCurrentProcess(), GetCurrentThread(), &s, &c, 0, SymFunctionTableAccess64, SymGetModuleBase64, 0))
			break;

		if (s.AddrReturn.Offset == 0 || s.AddrReturn.Offset == 0xffffffffcccccccc)
			break;

		if (_Offset)
		{
			_Offset--;
			continue;
		}

		_pSymbols[n++] = s.AddrReturn.Offset;
	}

	return n;
}
#else
size_t callstack(symbol* _pSymbols, size_t _NumSymbols, size_t _Offset)
{
	// + 1 skips this call to callstack
	return CaptureStackBackTrace(static_cast<ULONG>(_Offset + 1), static_cast<ULONG>(_NumSymbols), (PVOID*)_pSymbols, nullptr);
}
#endif

bool sdk_path(char* _Path, size_t _SizeofPath, const char* _SDKRelativePath)
{
	bool result = false;
	DWORD len = GetEnvironmentVariableA("ProgramFiles", _Path, static_cast<DWORD>(_SizeofPath));
	if (len && len < _SizeofPath)
	{
		strlcat(_Path, _SDKRelativePath, _SizeofPath);
		result = GetFileAttributesA(_Path) != INVALID_FILE_ATTRIBUTES;
	}

	return result;
}

template<size_t size> inline BOOL sdk_path(char (&_Path)[size], const char* _SDKRelativePath) { return sdk_path(_Path, size, _SDKRelativePath); }

static BOOL CALLBACK load_module(PCSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	//const oWinDbgHelp* d = static_cast<const oWinDbgHelp*>(UserContext);
	HANDLE hProcess = (HANDLE)UserContext;
	bool success = !!SymLoadModule64(hProcess, 0, ModuleName, 0, ModuleBase, ModuleSize);
	//if (d->GetModuleLoadedHandler())
	//	(*d->GetModuleLoadedHandler())(ModuleName, success);
	return success;
}

static HMODULE init_dbghelp(HANDLE _hProcess
	, bool _SoftLink
	, const char* _SymbolPath
	, char* _OutSymbolSearchPath
	, size_t _SizeofOutSymbolSearchPath)
{
	HMODULE hDbgHelp = nullptr;

	if (!_hProcess || _hProcess == INVALID_HANDLE_VALUE)
		_hProcess = GetCurrentProcess();

	char path[512];
	if (_SoftLink)
	{
		GetModuleFileNameA(0, path, oCOUNTOF(path));
		if (!GetLastError())
		{
			// first local override
			strlcat(path, ".local");
			if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
			{
				// then for an installed version (32/64-bit)
				if (sdk_path(path, "/Debugging Tools for Windows/dbghelp.dll") && GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
					hDbgHelp = LoadLibraryA(path);

				if (!hDbgHelp && sdk_path(path, "/Debugging Tools for Windows 64-Bit/dbghelp.dll") && GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
					hDbgHelp = LoadLibraryA(path);
			}
		}

		// else punt to wherever the system can find it
		if (!hDbgHelp)
			hDbgHelp = LoadLibraryA("dbghelp.dll");
	}

	if (hDbgHelp || !_SoftLink)
	{
		if (_OutSymbolSearchPath)
			*_OutSymbolSearchPath = '\0';

		// Our PDBs no longer have absolute paths, so we set the search path to
		// the executable path, unless the user specified _SymbolPath
		if (!_SymbolPath || !*_SymbolPath)
		{
			GetModuleFileNameA(0, path, oCOUNTOF(path));
			*(rstrstr(path, "\\") + 1) = '\0'; // trim filename
		}
		else
			strlcpy(path, _SymbolPath);

		if (SymInitialize(_hProcess, path, FALSE))
		{
			SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_FAIL_CRITICAL_ERRORS);
			if (_OutSymbolSearchPath)
				SymGetSearchPath(_hProcess, _OutSymbolSearchPath, static_cast<DWORD>(_SizeofOutSymbolSearchPath));

			EnumerateLoadedModules64(_hProcess, &load_module, _hProcess);
		}
		else
			oTHROW0(io_error);
	}

	return hDbgHelp;
}

symbol_info translate(symbol _Symbol)
{
	#ifdef _WIN64
		#define oIMAGEHLP_MODULE IMAGEHLP_MODULE64
		#define oIMAGEHLP_SYMBOL IMAGEHLP_SYMBOL64
		#define oIMAGEHLP_LINE IMAGEHLP_LINE64
		#define oSymGetModuleInfo SymGetModuleInfo64
		#define oSymGetSymFromAddr SymGetSymFromAddr64
		#define oSymGetLineFromAddr SymGetLineFromAddr64
		#define oDWORD DWORD64
	#else
		#define oIMAGEHLP_MODULE IMAGEHLP_MODULE
		#define oIMAGEHLP_SYMBOL IMAGEHLP_SYMBOL
		#define oIMAGEHLP_LINE IMAGEHLP_LINE
		#define oSymGetModuleInfo SymGetModuleInfo
		#define oSymGetSymFromAddr SymGetSymFromAddr
		#define oSymGetLineFromAddr SymGetLineFromAddr
		#define oDWORD DWORD
	#endif

	symbol_info si;
	si.address = _Symbol;

	if (!si.address)
	{
		si.module = "(null)";
		si.filename = "(null)";
		si.name = "(null)";
		si.symbol_offset = 0;
		si.line = 0;
		si.char_offset = 0;
		return si;
	}

	oIMAGEHLP_MODULE module;
	memset(&module, 0, sizeof(module));
	module.SizeOfStruct = sizeof(module);

	if (!oSymGetModuleInfo(GetCurrentProcess(), _Symbol, &module))
	{
		init_dbghelp(nullptr, false, nullptr, nullptr, 0);
		oVB(oSymGetModuleInfo(GetCurrentProcess(), _Symbol, &module));
	}
	
	si.module = module.ModuleName;
	BYTE buf[sizeof(IMAGEHLP_SYMBOL64) + mstring::Capacity * sizeof(TCHAR)];
	oIMAGEHLP_SYMBOL* symbolInfo = (oIMAGEHLP_SYMBOL*)buf;
	memset(buf, 0, sizeof(oIMAGEHLP_SYMBOL));
	symbolInfo->SizeOfStruct = sizeof(oIMAGEHLP_SYMBOL);
	symbolInfo->MaxNameLength = static_cast<DWORD>(si.name.capacity());

	oDWORD displacement = 0;
	oVB(oSymGetSymFromAddr(GetCurrentProcess(), _Symbol, &displacement, symbolInfo));

	// symbolInfo just contains the first 512 characters and doesn't guarantee
	// they will be null-terminated, so copy the buffer and ensure there's some
	// rational terminator
	//strcpy(si.name, symbolInfo->Name);
	memcpy(si.name, symbolInfo->Name, si.name.capacity() - sizeof(TCHAR));
	ellipsize(si.name);
	si.symbol_offset = static_cast<unsigned int>(displacement);

 	oIMAGEHLP_LINE line;
	memset(&line, 0, sizeof(line));
	line.SizeOfStruct = sizeof(line);

	DWORD disp = 0;
	if (oSymGetLineFromAddr(GetCurrentProcess(), _Symbol, &disp, &line))
	{
		si.filename = line.FileName;
		si.line = line.LineNumber;
		si.char_offset = disp;
	}

	else
	{
		si.filename.clear();
		si.line = 0;
		si.char_offset = 0;
	}

	return si;
}

static bool is_std_bind_impl_detail(const char* _Symbol)
{
	static const char* stdbindstrs[] = { "std::tr1::_Pmf", "std::tr1::_Callable_", "std::tr1::_Bind", "std::tr1::_Impl", "std::tr1::_Function", };
	static size_t stdbindlens[] = { 14, 20, 15, 15, 19, };
	static_assert(oCOUNTOF(stdbindstrs) == oCOUNTOF(stdbindlens), "");
	oFORI(i, stdbindstrs)
		if (!memcmp(_Symbol, stdbindstrs[i], stdbindlens[i]))
			return true;
	return false;
}

int format(char* _StrDestination, size_t _SizeofStrDestination, symbol _Symbol, const char* _Prefix, bool* _pIsStdBind)
{
	int rv = 0;
	symbol_info s = translate(_Symbol);
	if (_pIsStdBind && is_std_bind_impl_detail(s.name))
	{
		if (!*_pIsStdBind)
		{
			rv = snprintf(_StrDestination, _SizeofStrDestination, "%s... std::bind ...\n", _Prefix);
			*_pIsStdBind = true;
		}
	}

	else
	{
		if (_pIsStdBind)
			*_pIsStdBind = false;

		path fn_ = s.filename.filename();
		const char* fn = fn_.empty() ? "unknown" : fn_.c_str();

		if (s.line && s.char_offset)
			rv = snprintf(_StrDestination, _SizeofStrDestination, "%s%s!%s() ./%s Line %i + 0x%0x bytes\n", _Prefix, s.module.c_str(), s.name.c_str(), fn, s.line, s.char_offset);
		else if (s.line)
			rv = snprintf(_StrDestination, _SizeofStrDestination, "%s%s!%s() ./%s Line %i\n", _Prefix, s.module.c_str(), s.name.c_str(), fn, s.line);
		else
			rv = snprintf(_StrDestination, _SizeofStrDestination, "%s%s!%s() ./%s\n", _Prefix, s.module.c_str(), s.name.c_str(), fn);
	}

	return rv;
}

void print_callstack(char* _StrDestination, size_t _SizeofStrDestination, size_t _Offset, bool _FilterStdBind)
{
	int res = 0;
	size_t len = 0;

	size_t nSymbols = 0;
	*_StrDestination = 0;
	symbol address = 0;
	bool IsStdBind = false;
	while (callstack(&address, 1, _Offset++))
	{
		if (nSymbols++ == 0) // if we have a callstack, label it
		{
			res = _snprintf_s(_StrDestination, _SizeofStrDestination, _TRUNCATE, "\nCall Stack:\n");
			if (res == -1) goto TRUNCATION;
			len += res;
		}

		bool WasStdBind = IsStdBind;
		res = format(&_StrDestination[len], _SizeofStrDestination - len - 1, address, "", &IsStdBind);
		if (res == -1) goto TRUNCATION;
		len += res;

		if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
			_Offset += 5;
	}

	return;

TRUNCATION:
	static const char* kStackTooLargeMessage = "\n... truncated ...";
	size_t TLMLength = strlen(kStackTooLargeMessage);
	snprintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
}

static int write_dump_file(MINIDUMP_TYPE _Type, HANDLE _hFile, bool* _pSuccess, EXCEPTION_POINTERS* _pExceptionPointers)
{
	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = _pExceptionPointers;
	ExInfo.ClientPointers = TRUE; // true because we're in THIS process, this might need to change if this is called from another process.
	*_pSuccess = !!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), _hFile, _Type, _pExceptionPointers ? &ExInfo : nullptr, nullptr, nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}

bool dump(const path& _Path, bool _Full, void* _Exceptions)
{
	// Use most-direct APIs for this so there's less chance another crash/assert
	// can occur.

	filesystem::create_directories(_Path.parent_path());
	HANDLE hFile = CreateFileA(_Path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	const MINIDUMP_TYPE kType = _Full ? MiniDumpWithFullMemory : MiniDumpNormal;
	bool success = false; 
	if (_Exceptions)
		write_dump_file(kType, hFile, &success, (EXCEPTION_POINTERS*)_Exceptions);
	else
	{
		// If you're here, especially from a dump file, it's because the file was 
		// dumped outside an exception handler. In order to get stack info, we need
		// to cause an exception. See Remarks section:
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680360(v=vs.85).aspx

		// So from here, somewhere up the stack should be the line of code that 
		// triggered execution of this
		const static DWORD FORCE_EXCEPTION_FOR_CALLSTACK_INFO = 0x1337c0de;
		__try { RaiseException(FORCE_EXCEPTION_FOR_CALLSTACK_INFO, 0, 0, nullptr); }
		__except(write_dump_file(kType, hFile, &success, GetExceptionInformation())) {}
	}

	CloseHandle(hFile);
	return success;
}

static sstring make_dump_filename()
{
	sstring DumpFilename = this_module::get_path().basename();
	DumpFilename += "_v";
	
	module::info mi = this_module::get_info();
	to_string(&DumpFilename[1], DumpFilename.capacity() - 1, mi.version);
	DumpFilename += "D";

	ntp_date now;
	system::now(&now);
	sstring StrNow;
	strftime(DumpFilename.c_str() + DumpFilename.length()
		, DumpFilename.capacity() - DumpFilename.length()
		, syslog_local_date_format
		, now
		, date_conversion::to_local);

	replace(StrNow, DumpFilename, ":", "_");
	replace(DumpFilename, StrNow, ".", "_");

	return DumpFilename;
}

void dump_and_terminate(void* _exception, const char* _Message)
{
	sstring DumpFilename = make_dump_filename();

	bool Mini = false;
	bool Full = false;

	path_string DumpPath;
	const path& MiniDumpPath = windows::exception::mini_dump_path();
	if (!MiniDumpPath.empty())
	{
		snprintf(DumpPath, "%s%s.dmp", MiniDumpPath.c_str(), DumpFilename.c_str());
		Mini = dump(path(DumpPath), false, _exception);
	}

	const path& FullDumpPath = windows::exception::full_dump_path();
	if (!FullDumpPath.empty())
	{
		snprintf(DumpPath, "%s%s.dmp", FullDumpPath.c_str(), DumpFilename.c_str());
		Full = dump(path(DumpPath), true, _exception);
	}

	const char* PostDumpCommand = windows::exception::post_dump_command();

	if (PostDumpCommand && *PostDumpCommand)
		::system(PostDumpCommand);

	if (windows::exception::prompt_after_dump())
	{
		xlstring msg;
		snprintf(msg, "%s\n\nThe program will now exit.%s"
			, _Message
			, Mini || Full ? "\n\nA .dmp file has been written." : "");

		MessageBox(NULL, msg.c_str(), this_module::get_path().c_str(), MB_ICONERROR|MB_OK|MB_TASKMODAL); 
	}
	std::exit(-1);
}

	} // namespace debugger
} // namespace ouro
