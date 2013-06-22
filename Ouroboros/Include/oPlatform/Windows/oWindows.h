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

// This should be included instead of, or at least before <windows.h> to avoid
// much of the conflict-causing overhead as well as include some useful 
#pragma once
#ifndef oWindows_h
#define oWindows_h
#if defined(_WIN32) || defined(_WIN64)

#ifdef _WINDOWS_
	#pragma message("BAD WINDOWS INCLUDE! Applications should #include <oWindows.h> to prevent extra and sometimes conflicting cruft from being included.")
#endif

#include <oStd/guid.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>
#include <oBasis/oVersion.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oModule.h> // for module-to-platform code in the Misc section

// _____________________________________________________________________________
// Simplify the contents of Windows.h

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#define NOATOM
#define NOCOMM
#define NOCRYPT
#define NODEFERWINDOWPOS
//#define NODRAWTEXT // used in oWindow.h
#define NOGDICAPMASKS
#define NOHELP
#define NOIMAGE
#define NOKANJI
#define NOKERNEL
#define NOMCX
#define NOMEMMGR
//#define NOMENUS // we support creating menus in oWinWindowing.h
#define NOMETAFILE
#define NOOPENFILE
#define NOPROFILER
#define NOPROXYSTUB
//#define NORASTEROPS // used in oWindow.h
#define NORPC
#define NOSCROLL
//#define NOSERVICE // used below by services queries
#define NOSOUND
#define NOTAPE
#define WIN32_LEAN_AND_MEAN

#ifdef interface
	#undef interface
	#undef INTERFACE_DEFINED
#endif
#undef _M_CEE_PURE

// Including intrin is necessary for interlocked functions, but it causes
// a warning to spew from math. So squelch that warning since it seems the
// declarations are indeed the same.
#include <intrin.h>
#pragma warning(disable:4985) // 'ceil': attributes not present on previous declaration.
#include <math.h>
#pragma warning (default:4985)

#include <windows.h>
#include <winsock2.h>

#define oDXVER_9a 0x0900
#define oDXVER_9b 0x0901
#define oDXVER_9c 0x0902
#define oDXVER_10 0x1000
#define oDXVER_10_1 0x1001
#define oDXVER_11 0x1100
#define oDXVER_11_1 0x1101
#define oDXVER_12 0x1200
#define oDXVER_12_1 0x1201
#define oDXVER_13 0x1300
#define oDXVER_13_1 0x1301

// Some GPU drivers have bugs in newer features that we use, so ensure we're at
// least on this version and hope there aren't regressions.

#define oNVVER_MAJOR 285
#define oNVVER_MINOR 62

#define oAMDVER_MAJOR 8
#define oAMDVER_MINOR 982

// Define something like WINVER, but for DX functionality
#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oDXVER oDXVER_11
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN)) || (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	#define oDXVER oDXVER_11 //oDXVER_10_1
#else
	#define oDXVER oDXVER_9c
#endif

// mostly for debugging...
#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	oBUILD_TRACE("PLATFORM: Windows7")
	#define oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
	#define oWINDOWS_HAS_TRAY_QUIETTIME
	#define oWINDOWS_HAS_SHMUTEX_TRYLOCK
	#define oWINDOWS_HAS_REGISTERTOUCHWINDOW
#elif (defined(NTDDI_VISTA) && (NTDDI_VERSION >= NTDDI_VISTA))
	oBUILD_TRACE("PLATFORM: Windows Vista")
#elif (defined(NTDDI_LONGHORN) && (NTDDI_VERSION >= NTDDI_LONGHORN))
	oBUILD_TRACE("PLATFORM: Windows Longhorn")
#elif (defined(NTDDI_WINXP) && (NTDDI_VERSION >= NTDDI_WINXP))
	oBUILD_TRACE("PLATFORM: Windows XP")
#endif

#if (_MSC_VER >= 1600)
	oBUILD_TRACE("COMPILER: Visual Studio 2010")
#elif (_MSC_VER >= 1500)
	oBUILD_TRACE("COMPILER: Visual Studio 2008")
#endif

#if oDXVER >= oDXVER_13_1
	oBUILD_TRACE("DIRECTX: 13.1")
#elif oDXVER >= oDXVER_13
	oBUILD_TRACE("DIRECTX: 13.0")
#elif oDXVER >= oDXVER_12_1
	oBUILD_TRACE("DIRECTX: 12.1")
#elif oDXVER >= oDXVER_12
	oBUILD_TRACE("DIRECTX: 12.0")
#elif oDXVER >= oDXVER_11_1
	oBUILD_TRACE("DIRECTX: 11.1")
#elif oDXVER >= oDXVER_11
	oBUILD_TRACE("DIRECTX: 11.0")
#elif oDXVER >= oDXVER_10_1
	oBUILD_TRACE("DIRECTX: 10.1")
#elif oDXVER >= oDXVER_10
	oBUILD_TRACE("DIRECTX: 10.0")
#elif oDXVER >= oDXVER_9c
	oBUILD_TRACE("DIRECTX: 9.0c")
#elif oDXVER >= oDXVER_9b
	oBUILD_TRACE("DIRECTX: 9.0b")
#elif oDXVER >= oDXVER_9a
	oBUILD_TRACE("DIRECTX: 9.0a")
#endif

#include <shlobj.h>
#include <shellapi.h>

#if oDXVER >= oDXVER_10
	#include <dxerr.h>
	#include <d3d11.h>
	#include <d3dx11.h>
	#include <d3dcompiler.h>
	#include <dxgi.h>
	#include <d2d1.h>
	#include <dwrite.h>
#endif

#ifdef interface
	#define INTERFACE_DEFINED
#endif

// _____________________________________________________________________________
// Abstract between running with a console and not for release.
// Example:
//
// oMAIN() // will have a console for log output/stdout in debug, and no console in non-debug builds
// {
//   ExecuteMyProgram();
//   return 0;
// }

#define oMAINA() \
	int oMain__(int argc, const char* argv[]); \
	int oWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int (*oMain)(int argc, const char* argv[])); \
	int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { return oWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow, oMain__); } \
	int oMain__(int argc, const char* argv[])

#ifdef _DEBUG
	#define oMAIN() int main(int argc, const char* argv[])
#else
	#define oMAIN() oMAINA()
#endif

// _____________________________________________________________________________
// Error check wrappers. These interpret HRESULTS, so should not be used on 
// anything but WIN32 API. oVB is for the return FALSE then GetLastError() 
// pattern, and oV is for direct HRESULT return values.

#ifdef _DEBUG
	#define oVB(fn) do { if (!(fn)) { oWinSetLastError(::GetLastError()); oASSERT_TRACE(oStd::assert_type::assertion, oStd::assert_action::ignore, #fn, "%s", oErrorGetLastString()); } } while(false)
	#define oV(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { oWinSetLastError(HR__); oASSERT_TRACE(oStd::assert_type::assertion, oStd::assert_action::ignore, #fn, "%s", oErrorGetLastString()); } } while(false)
#else
	#define oVB(fn) fn
	#define oV(fn) fn
#endif

#define oWIN_CHECK_HR(_Fn, _Message, ...) do { HRESULT HR__ = _Fn; if (FAILED(HR__)) { throw std::system_error(std::make_error_code(oWinGetErrc(HR__)), oStd::formatf(_Message, ## __VA_ARGS__)); } } while(false)

// _____________________________________________________________________________
// Wrappers for the Windows-specific crtdbg API. Prefer oASSERT macros found
// in oAssert.h, but for a few systems that are lower-level than oAssert, these
// may be necessary.

#ifdef _DEBUG
	#include <crtdbg.h>

	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "OOOii Debug Library", #expr "\n\n" msg, ## __VA_ARGS__)) __debugbreak(); }
	#define oCRTWARNING(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", "WARNING: " msg, ## __VA_ARGS__); } while(false)
	#define oCRTTRACE(msg, ...) do { _CrtDbgReport(_CRT_WARN, __FILE__, __LINE__, "OOOii Debug Library", msg, ## __VA_ARGS__); } while(false)

	// Convenience wrapper for quick scoped leak checking
	class oLeakCheck
	{
		const char* Name;
		_CrtMemState StartState;
	public:
		oLeakCheck(const char* _ConstantName = "") : Name(_ConstantName ? _ConstantName : "(unnamed)") { _CrtMemCheckpoint(&StartState); }
		~oLeakCheck()
		{
			_CrtMemState endState, stateDiff;
			_CrtMemCheckpoint(&endState);
			_CrtMemDifference(&stateDiff, &StartState, &endState);
			oCRTTRACE("---- Mem diff for %s ----", Name);
			_CrtMemDumpStatistics(&stateDiff);
		}
	};
#else
	#define oCRTASSERT(expr, msg, ...) __noop
	#define oCRTWARNING(msg, ...) __noop
	#define oCRTTRACE(msg, ...) __noop
#endif

// _____________________________________________________________________________
// Declare functions that are in Windows DLLs, but not exposed in headers

// Secret function that is not normally exposed in headers.
// Typically pass 0 for wLanguageId, and specify a timeout
// for the dialog in milliseconds, returns MB_TIMEDOUT if
// the timeout is reached.
int MessageBoxTimeoutA(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
int MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef UNICODE
	#define MessageBoxTimeout MessageBoxTimeoutW
#else
	#define MessageBoxTimeout MessageBoxTimeoutA
#endif 

#define MB_TIMEDOUT 32000

// Return the closest std::errc for the specified HRESULT.
std::errc::errc oWinGetErrc(HRESULT _hResult);

// Given the specified HRESULT, set both the closest errno value and the 
// platform-specific description associated with the error code.
// if oWINDOWS_DEFAULT is specified, ::GetLastError() is used
bool oWinSetLastError(HRESULT _hResult = oDEFAULT, const char* _ErrorDescPrefix = 0);

// oV_RETURN executes a block of code that returns an HRESULT
// if the HRESULT is not S_OK it returns the HRESULT
#define oV_RETURN(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) return HR__; } while(false)

// Executes a Windows API that returns bool and populates oErrorGetLast() with 
// ::GetLastError() and returns false.
#define oVB_RETURN(fn) do { if (!(fn)) { return oWinSetLastError(oDEFAULT, #fn " failed: "); } } while(false)

// Executes a Windows API that returns HRESULT and on failure sets the Windows
// error code as the last error and returns false.
#define oVB_RETURN2(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { return oWinSetLastError(HR__, #fn " failed: "); } } while(false)

// Executes a Windows API that returns HRESULT and on failure sets the Windows
// error code as the last error and returns no value (use this in ctors).
#define oVB_RETURN3(fn) do { HRESULT HR__ = fn; if (FAILED(HR__)) { oWinSetLastError(HR__, #fn " failed: "); return; } } while(false)

// _____________________________________________________________________________
// Smart pointer support

inline ULONG oGetRefCount(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

// _____________________________________________________________________________
// Other conversion

inline const oGUID& oGetGUID(const GUID& _WinGUID) { return *reinterpret_cast<const oGUID*>(&_WinGUID); }

// An ASCII version of CommandLineToArgvW. Use oWinCommandLineToArgvAFree() to
// free the buffer returned by this function.
const char** oWinCommandLineToArgvA(bool _ExePathAsArg0, const char* pCmdLine, int* _pArgc);

// Frees a buffer returned by oWinCommandLineToArgvA
void oWinCommandLineToArgvAFree(const char** _pArgv);

inline float oPointToDIP(float _Point) { return 96.0f * _Point / 72.0f; }
inline float oDIPToPoint(float _DIP) { return 72.0f * _DIP / 96.0f; }

// Convert from values in VS_FIXEDFILEINFO to oVersion and back
void oWinGetVersion(const oVersion& _Version, DWORD* _pVersionMS, DWORD* _pVersionLS);
oVersion oWinGetVersion(DWORD _VersionMS, DWORD _VersionLS);

// _____________________________________________________________________________
// Concurrency

// LoadLibrary holds an internal mutex that can deadlock when OOOii lib executes
// code that loads another library. This has only been observed to happen in 
// early process initialization specifically with oProcessHeap, so use these to
// protect against deadlocks.
HMODULE oThreadsafeLoadLibrary(LPCTSTR _lpFileName);
BOOL oThreadsafeFreeLibrary(HMODULE _hModule);

// Prefer this over direct usage of OutputDebugString. NOTE: The mutex is
// process-wide, no matter what DLLs or how this code was linked.
void oThreadsafeOutputDebugStringA(const char* _OutputString);

// returns true if wait finished successfully, or false if
// timed out or otherwise errored out.
bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS = oInfiniteWait);

// If _pWaitBreakingIndex is nullptr, this waits for all handles to be 
// unblocked. If a valid pointer, then it is filled with the index of the handle
// that unblocked the wait.
bool oWaitMultiple(HANDLE* _pHandles, size_t _NumberOfHandles, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oInfiniteWait);

bool oWaitSingle(DWORD _ThreadID, unsigned int _Timeout = oInfiniteWait);
bool oWaitMultiple(DWORD* _pThreadIDs, size_t _NumberOfThreadIDs, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oInfiniteWait);

template<typename GetNativeHandleT> inline bool oTWaitMultiple(threadsafe GetNativeHandleT** _ppWaitable, size_t _NumWaitables, size_t* _pWaitBreakingIndex = nullptr, unsigned int _TimeoutMS = oInfiniteWait)
{
	HANDLE handles[64]; // 64 is a windows limit
	for (size_t i = 0; i < _NumWaitables; i++)
		handles[i] = static_cast<HANDLE>(_ppWaitable[i]->GetNativeHandle());
	return oWaitMultiple(handles, _NumWaitables, _pWaitBreakingIndex, _TimeoutMS);
}

// _____________________________________________________________________________
// Window utilties

enum oWINDOWS_VERSION
{
	oWINDOWS_UNKNOWN,
	oWINDOWS_2000,
	oWINDOWS_XP,
	oWINDOWS_XP_PRO_64BIT,
	oWINDOWS_SERVER_2003,
	oWINDOWS_HOME_SERVER,
	oWINDOWS_SERVER_2003R2,
	oWINDOWS_VISTA,
	oWINDOWS_SERVER_2008,
	oWINDOWS_VISTA_SP1,
	oWINDOWS_SERVER_2008_SP1,
	oWINDOWS_VISTA_SP2,
	oWINDOWS_SERVER_2008_SP2,
	oWINDOWS_7,
	oWINDOWS_SERVER_2008R2,
	oWINDOWS_7_SP1,
	oWINDOWS_SERVER_2008R2_SP1,
	oWINDOWS_8,
	oWINDOWS_SERVER_2012,
	oWINDOWS_8_SP1,
	oWINDOWS_SERVER_2012_SP1,
};

oWINDOWS_VERSION oGetWindowsVersion();

// If the windows version is not at least the specfied version
// an error message will be thrown up and the application will
// terminate
void oVerifyMinimumWindowsVersion(oWINDOWS_VERSION _Version);
bool oIsWindows64Bit();

bool oIsAeroEnabled();
bool oEnableAero(bool _Enabled, bool _Force = false);

bool oIsRemoteDesktopConnected();

// Some Windows API take string blocks, that is one buffer of 
// nul-separated strings that end in a nul terminator. This is
// often not convenient to construct, so allow construction in
// another way and use this to convert.
bool oConvertEnvStringToEnvBlock(char* _EnvBlock, size_t _SizeofEnvBlock, const char* _EnvString, char _Delimiter);
template<size_t size> inline bool oConvertEnvStringToEnvBlock(char (&_EnvBlock)[size], const char* _EnvString, char _Delimiter) { return oConvertEnvStringToEnvBlock(_EnvBlock, size, _EnvString, _Delimiter); }

HICON oIconFromBitmap(HBITMAP _hBmp);

// @oooii-tony: A wrapper for SetWindowsHookEx that includes a user-specified context.
// (Is there a way to do this?! Please someone let me know!)
typedef LRESULT (CALLBACK* oHOOKPROC)(int _nCode, WPARAM _wParam, LPARAM _lParam, void* _pUserData);
bool oUnhookWindowsHook(HHOOK _hHook);
HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, DWORD _dwThreadId);
// Use this form to hook a window in the current process.
inline HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, HWND _hWnd) { return oSetWindowsHook(_idHook, _pHookProc, _pUserData, GetWindowThreadProcessId(_hWnd, 0)); }

// _____________________________________________________________________________
// Dialog box helpers

enum oWINDOWS_DIALOG_ITEM_TYPE
{
	oDLG_BUTTON,
	oDLG_EDITBOX,
	oDLG_LABEL_LEFT_ALIGNED,
	oDLG_LABEL_CENTERED,
	oDLG_LABEL_RIGHT_ALIGNED,
	oDLG_LARGELABEL,
	oDLG_ICON,
	oDLG_LISTBOX,
	oDLG_SCROLLBAR,
	oDLG_COMBOBOX,
};

struct oWINDOWS_DIALOG_ITEM
{
	const char* Text;
	oWINDOWS_DIALOG_ITEM_TYPE Type;
	WORD ItemID;
	RECT Rect;
	bool Enabled;
	bool Visible;
	bool TabStop;
};

struct oWINDOWS_DIALOG_DESC
{
	const char* Font;
	const char* Caption;
	const oWINDOWS_DIALOG_ITEM* pItems;
	UINT NumItems;
	UINT FontPointSize;
	RECT Rect;
	bool Center; // if true, ignores Rect.left, Rect.top positioning
	bool SetForeground;
	bool Enabled;
	bool Visible;
	bool AlwaysOnTop;
};

LPDLGTEMPLATE oDlgNewTemplate(const oWINDOWS_DIALOG_DESC& _Desc);
void oDlgDeleteTemplate(LPDLGTEMPLATE _lpDlgTemplate);

bool oWinEnumVideoDriverDesc(oFUNCTION<void(const oDISPLAY_ADAPTER_DRIVER_DESC& _pDesc)> _Enumerator);

// _____________________________________________________________________________
// System API - dealing with Windows as a whole

// Goes through all services/drivers currently on the system
bool oWinServicesEnum(oFUNCTION<bool(SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status)> _Enumerator);

// Returns the resolved path to the binary that is the running service
bool oWinGetServiceBinaryPath(char* _StrDestination, size_t _SizeofStrDestination, SC_HANDLE _hSCManager, const char* _ServiceName);

// Returns true if the specified file is a binary compiled for x64. If false,
// it is reasonable to assume x86 (32-bit).
bool oWinSystemIs64BitBinary(const char* _StrPath);

// Returns true if all services on the system are not in a pending state.
bool oWinSystemAllServicesInSteadyState();

// Returns a percent [0,100] of CPU usage across all processes and all cores
// (basically the summary percentage that Task Manager gives you). The value
// returned is over a sample period, so it is required that two values are 
// cached to be compared against for the calculation. Thus this function should
// be called as a regular interval for refreshing the current CPU usage.
double oWinSystemCalculateCPUUsage(unsigned long long* _pPreviousIdleTime, unsigned long long* _pPreviousSystemTime);

// Spawns the application associated with the specified _DocumentName and opens
// that document. For example: open a text file using Notepad or a URL using 
// Explorer.
bool oWinSystemOpenDocument(const char* _DocumentName, bool _ForEdit = false);

// Retrieves the HWND of the top level window owned by the specified process and
// the ID of the thread that services this window.  Since a process can have more
// than one top level window an optional name can also be specified to make certain
// the correct window is returned
bool oWinGetProcessTopWindowAndThread(unsigned int _ProcessID, HWND* _pHWND, unsigned int* _pThreadID, const char* _pOptionalWindowName = nullptr);

// _____________________________________________________________________________
// Identification/ID Conversion API

// Converts a standard C file handle from fopen to a windows handle
HANDLE oGetFileHandle(FILE* _File);

DWORD oGetThreadID(HANDLE _hThread);

// Fills the specified buffer with the name of the workgroup for the system
bool oWinGetWorkgroupName(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> bool oWinGetWorkgroupName(char (&_StrDestination)[size]) { return oWinGetWorkgroupName(_StrDestination, size); }

// Returns the current module. Unlike GetModuleHandle(NULL), this returns the
// module handle containing the specified pointer. A typical use case would be
// to pass the address of the function calling oGetModule() to return the 
// current module at that point. If NULL is specified, this returns the handle
// of the main process module.
HMODULE oGetModule(void* _ModuleFunctionPointer);

// Returns true if _pOutProcessID has been filled with the ID of the specified
// process image name (exe name). If this returns false, check oErrorGetLast()
// for more information.
bool oWinGetProcessID(const char* _Name, DWORD* _pOutProcessID);

// Call the specified function for each thread in the current process. The 
// function should return true to keep enumerating, or false to exit early. This
// function returns false if there is a failure, check oErrorGetLast() for more
// information.
bool oEnumProcessThreads(DWORD _ProcessID, oFUNCTION<bool(DWORD _ThreadID, DWORD _ParentProcessID)> _Function);

// Call the specified function for each of the child processes of the current
// process. The function should return true to keep enumerating, or false to
// exit early. This function returns false if there is a failure, check 
// oErrorGetLast() for more information. The error can be ECHILD if there are no 
// child processes.
bool oWinEnumProcesses(oFUNCTION<bool(DWORD _ProcessID, DWORD _ParentProcessID, const char* _ProcessExePath)> _Function);

// Call the specified function for each of the top level windows on the system. 
// The function should return true to keep searching or false to exit early.
bool oWinEnumWindows(oFUNCTION<bool(HWND _Hwnd)> _Function);

// _____________________________________________________________________________
// Misc

// Uses a waitable timer to call the specified function at the specified time.
// If _Alertable is true, this can wake up a sleeping system.
bool oScheduleTask(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oTASK _Task);

// Adjusts privileges and calls ExitWindowsEx with the specified parameters
bool oWinExitWindows(UINT _uFlags, DWORD _dwReason);

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY);

// Returns the display index as used by EnumDisplayDevices and the populated
// DISPLAY_DEVICE for the specified _hMonitor
int oWinGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice);

void oGetVirtualDisplayRect(RECT* _pRect);

inline POINT oAsPOINT(const int2& _P) { POINT p; p.x = _P.x; p.y = _P.y; return p; }

// You can't allocate large memory pages by default. You have to enable privileges. This will fail if the app does not have permission in the local security policy, 
// You will probably have to run with uac disabled, or run the app with elevated privileges as well.
bool oWinAddLargePagePrivileges();

bool oWinSetPrivilege(HANDLE _hProcessToken, LPCTSTR _PrivilegeName, bool _Enabled);

// Call this to allow this process to be debugged
bool oWinEnableDebugPrivilege(bool _Enabled);

// Call this to dump the stack and potentially throw up an error message if so enabled by oReporting.  See oReporting for more information on stack dumps.
void oWinDumpAndTerminate(EXCEPTION_POINTERS* _pExceptionPtrs, const char* _pUserErrorMessage);

// value to set for FILEFLAGS in a resource script (.rc)
long oWinRCGetFileFlags(const oMODULE_DESC& _Desc);

// Returns the values for FILETYPE and FILESUBTYPE in a resource script (.rc)
void oWinRCGetFileType(const oMODULE_TYPE _Type, DWORD* _pType, DWORD* _pSubtype);

#else
	#error Unsupported platform
#endif

#endif
