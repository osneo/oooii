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
#include <oPlatform/oFileSchemeHandler.h>
#include <oConcurrency/mutex.h>
#include "oDispatchQueueGlobalIOCP.h"
#include "oIOCP.h"
#include <oCore/windows/win_error.h>

using namespace ouro;
using namespace oConcurrency;

#define oTRACE_MONITOR oTRACE

enum oSYSPATH
{
	oSYSPATH_CWD, // current working directory
	oSYSPATH_APP, // application directory (path where exe is)
	oSYSPATH_APP_FULL, // full path (with filename) to application executable
	oSYSPATH_SYSTMP, // platform temporary directory
	oSYSPATH_SYS, // platform system directory
	oSYSPATH_OS, // platform installation directory
	oSYSPATH_DEV, // current project development root directory
	oSYSPATH_TESTTMP, // unit test temp path. destroyed if unit tests succeed
	oSYSPATH_COMPILER_INCLUDES, // location of compiler includes
	oSYSPATH_DESKTOP, // platform current user desktop
	oSYSPATH_DESKTOP_ALLUSERS, // platform shared desktop
	oSYSPATH_SCCROOT, // the root of the source control under which this code runs
	oSYSPATH_DATA, // the data path of the application
};

inline bool oIsSeparator(int _Char) { return _Char == '\\' || _Char == '/'; }
char* oEnsureSeparator(char* _Path, size_t _SizeofPath, char _FileSeparator = '/')
{
	size_t len = strlen(_Path);
	char* cur = _Path + len-1;
	if (!oIsSeparator(*cur) && _SizeofPath)
	{
		oASSERT((len+1) < _SizeofPath, "Path string does not have the capacity to have a separate appended (%s)", oSAFESTRN(_Path));
		*(++cur) = _FileSeparator;
		*(++cur) = 0;
	}

	return _Path;
}

char* oSystemURIPartsToPath(char* _Path, size_t _SizeofPath, const oURIParts& _URIParts)
{
	if (_URIParts.Authority.empty())
	{
		if (_URIParts.Path[0]=='/' && _URIParts.Path[2]==':')
			strlcpy(_Path, _URIParts.Path+1, _SizeofPath);
		else
			strlcpy(_Path, _URIParts.Path, _SizeofPath);
	}
	else
	{
		oSYSPATH SysPath;
		if (from_string(&SysPath, _URIParts.Authority))
		{
			path path;
			switch (SysPath)
			{
				case oSYSPATH_CWD: path = ouro::filesystem::current_path(); break;
				case oSYSPATH_APP: path = ouro::filesystem::app_path(); break;
				case oSYSPATH_SYSTMP: path = ouro::filesystem::temp_path(); break;
				case oSYSPATH_SYS: path = ouro::filesystem::system_path(); break;
				case oSYSPATH_OS: path = ouro::filesystem::os_path(); break;
				case oSYSPATH_DEV: path = ouro::filesystem::dev_path(); break;
				case oSYSPATH_TESTTMP: path = ouro::filesystem::temp_path(); break;
				case oSYSPATH_DESKTOP: path = ouro::filesystem::desktop_path(); break;
				case oSYSPATH_DESKTOP_ALLUSERS: path = ouro::filesystem::desktop_path(); break;
				case oSYSPATH_DATA: path = ouro::filesystem::data_path(); break;
				default: oErrorSetLast(std::errc::protocol_error, "Failed to find %s", as_string(SysPath)); return nullptr;
			}

			if (strlcpy(_Path, path, _SizeofPath) >= _SizeofPath)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}

			if (!oEnsureSeparator(_Path, _SizeofPath) && oErrorGetLast() != std::errc::operation_in_progress)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}

			if (-1 == sncatf(_Path, _SizeofPath, _URIParts.Path))
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}
		}

		else if (!oURIPartsToPath(_Path, _SizeofPath, _URIParts))
		{
			oErrorSetLast(std::errc::invalid_argument);
			return nullptr;
		}
	}

	return _Path;
}
template<size_t size> char* oSystemURIPartsToPath(char (&_ResultingFullPath)[size], const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, size, _URIParts); }
template<size_t capacity> char* oSystemURIPartsToPath(ouro::fixed_string<char, capacity>& _ResultingFullPath, const oURIParts& _URIParts) { return oSystemURIPartsToPath(_ResultingFullPath, _ResultingFullPath.capacity(), _URIParts); }

static bool oFileGetDesc(const path& _Path, oSTREAM_DESC* _pDesc)
{
	try
	{
		auto status = ouro::filesystem::status(_Path);
		if (!ouro::filesystem::exists(status))
			return false;
		_pDesc->Created = 0;
		_pDesc->Accessed = 0;
		_pDesc->Written = ouro::filesystem::last_write_time(_Path);
		_pDesc->Size = ouro::filesystem::file_size(_Path);
		_pDesc->Directory = ouro::filesystem::is_directory(status);
		_pDesc->Hidden = false;
		_pDesc->ReadOnly = ouro::filesystem::is_read_only(status);
	}

	catch (std::exception&)
	{
		return false;
	}
	return true;
}

namespace ouro {

	const char* as_string(const oSYSPATH& _SysPath)
	{
		switch (_SysPath)
		{
			case oSYSPATH_CWD: return "oSYSPATH_CWD";
			case oSYSPATH_APP: return "oSYSPATH_APP";
			case oSYSPATH_APP_FULL: return "oSYSPATH_APP_FULL";
			case oSYSPATH_SYSTMP: return "oSYSPATH_SYSTMP";
			case oSYSPATH_SYS: return "oSYSPATH_SYS";
			case oSYSPATH_OS: return "oSYSPATH_OS";
			case oSYSPATH_DEV: return "oSYSPATH_DEV";
			case oSYSPATH_TESTTMP: return "oSYSPATH_TESTTMP";
			case oSYSPATH_COMPILER_INCLUDES: return "oSYSPATH_COMPILER_INCLUDES";
			case oSYSPATH_DESKTOP: return "oSYSPATH_DESKTOP";
			case oSYSPATH_DESKTOP_ALLUSERS: return "oSYSPATH_DESKTOP_ALLUSERS";
			case oSYSPATH_DATA: return "oSYSPATH_DATA";
			oNODEFAULT;
		}
	}

bool from_string(oSYSPATH* _pValue, const char* _StrSource)
{
	static const char* sStrings[] =
	{
		"oSYSPATH_CWD",
		"oSYSPATH_APP",
		"oSYSPATH_APP_FULL",
		"oSYSPATH_SYSTMP",
		"oSYSPATH_SYS",
		"oSYSPATH_OS",
		"oSYSPATH_DEV",
		"oSYSPATH_TESTTMP",
		"oSYSPATH_COMPILER_INCLUDES",
		"oSYSPATH_DESKTOP",
		"oSYSPATH_DESKTOP_ALLUSERS",
		"oSYSPATH_P4ROOT",
		"oSYSPATH_DATA",
	};

	sstring SourceUppercase = _StrSource;
	toupper(SourceUppercase);
	oFORI(i, sStrings)
	{
		if (!strcmp(_StrSource, sStrings[i]) || !strcmp(SourceUppercase, sStrings[i]+9)) // +9 match against just "OS" or "HOST" after oSYSPATH_
		{
			*_pValue = (oSYSPATH)i;
			return true;
		}
	}
	return false;
}

} // namespace ouro

static void oSetHighLowOffset(long& _low, long& _high, unsigned long long _offset)
{
	_low = _offset & 0xffffffffull;
	_high = _offset >> 32ull;
}

struct oFileReaderImpl : public oStreamReader
{
	static const int UNBUFFERED_ALIGN_REQUIREMENT = 4096;

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	
	oFileReaderImpl(const oURIParts& _URIParts, bool _Unbuffered, bool* _pSuccess)
		: URIParts(_URIParts)
		, Unbuffered(_Unbuffered)
		, hFile(nullptr)
		, EoF(false)
	{
		*_pSuccess = false;

		path_string Path;
		oSystemURIPartsToPath(Path, _URIParts);
		hFile = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, Unbuffered ? FILE_FLAG_NO_BUFFERING : 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			switch (GetLastError())
			{
				case ERROR_FILE_NOT_FOUND:
				{
					uri_string URIRef;
					oVERIFY(oURIRecompose(URIRef, _URIParts));
					oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", URIRef.c_str());
					break;
				}

				case ERROR_INVALID_NAME:
				{
					uri_string URIRef;
					oVERIFY(oURIRecompose(URIRef, _URIParts));
					oErrorSetLast(std::errc::invalid_argument, "invalid URI: %s", URIRef.c_str());
					break;
				}

				default:
					break;
			}

			return;
		}
		else
		{
			oSTREAM_DESC FDesc;
			oFileGetDesc(path(Path), &FDesc);
			Desc.Initialize(FDesc);

			if (!oDispatchQueueCreateGlobalIOCP("File reader dispatch queue", 10, &ReadQueue))
			{
				oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP dispatch queue.");
				CloseHandle(hFile);
				return;
			}
		}

		*_pSuccess = true;
	}

	~oFileReaderImpl()
	{
		if(ReadQueue)
		{
			ReadQueue->Join();
			oASSERT(!ReadQueue->Joinable(), "The ReadQueue should have joined! What is running that prevented it from joining?");
		}

		Close();
	}

	void DispatchRead(const oSTREAM_READ& _StreamRead, continuation_t _Continuation) threadsafe override
	{
		unsigned long long fileSize = Desc->Size;
		if(Unbuffered)
			fileSize = byte_align(fileSize, UNBUFFERED_ALIGN_REQUIREMENT); // unbuffered io is always a multiple of the alignment
		
		if ((_StreamRead.Range.Offset + _StreamRead.Range.Size) <= fileSize)
		{
			intrusive_ptr<threadsafe oFileReaderImpl> self(this);
			// Doing it this way because standard windows async file io is not 
			// GUARANTEED to be asynchronous. Windows can decided to do the read 
			// synchronously on its own, so FORCE async this way.
			ReadQueue->Dispatch([self, _StreamRead, _Continuation]() mutable {
				bool success = self->Read(_StreamRead);
				_Continuation(success, self.c_ptr(), _StreamRead);
			});
		}
		else
		{
			oErrorSetLast(std::errc::io_error, "Specified range will read past file size");
			_Continuation(false, this, _StreamRead);
			return;
		}
	}

	bool Read(const oSTREAM_READ& _StreamRead) threadsafe
	{
		oASSERT(!Unbuffered || (byte_aligned(_StreamRead.pData, UNBUFFERED_ALIGN_REQUIREMENT) && 
			byte_aligned(_StreamRead.Range.Offset, UNBUFFERED_ALIGN_REQUIREMENT) && byte_aligned(_StreamRead.Range.Size, UNBUFFERED_ALIGN_REQUIREMENT)), 
			"Unbuffered io has very restrict requirements. file offset, read size, and pointer address must be 4k aligned");

		DWORD numBytesRead;
		long offsetLow, offsetHigh;
		oSetHighLowOffset(offsetLow, offsetHigh, _StreamRead.Range.Offset);

		// Need to lock since SetFilePointer/ReadFile are separate calls. And to play nice with close
		lock_guard<mutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(std::errc::operation_canceled);
		}

		SetFilePointer(hFile, offsetLow, &offsetHigh, FILE_BEGIN);
		EoF = false; // because of file pointer move
		oVB(ReadFile(hFile, _StreamRead.pData, as_uint(_StreamRead.Range.Size), &numBytesRead, nullptr));
		EoF = (numBytesRead == 0);

		// Unbuffered io is inconsistent on what it returns for these numbers and 
		// it doesn't really apply for anyway because unbuffered always physically 
		// reads in an even number of sectors.
		if (!Unbuffered && numBytesRead != _StreamRead.Range.Size) // happens when trying to read past end of file, but ReadFile itself still succeeds
		{
			// failed, but not a bug if reading past end of file. 
			oASSERT(_StreamRead.Range.Offset + _StreamRead.Range.Size >= Desc->Size, "ReadFile didn't read enough bytes, but didn't read past end of file");
			return false;
		}

		return true;
	}

	bool EndOfFile() threadsafe const override { return EoF; }

	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override
	{
		*_pDesc = *Desc;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		lock_guard<mutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oSTREAM_DESC> Desc;
	intrusive_ptr<threadsafe oDispatchQueueGlobal> ReadQueue;
	mutex Mutex;
	oInitOnce<oURIParts> URIParts;
	HANDLE hFile;
	oRefCount RefCount;
	bool Unbuffered;
	bool EoF;
};

struct oFileWriterImpl : public oStreamWriter
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	struct Operation
	{
		oSTREAM_WRITE Write;
		continuation_t Continuation;
	};

	oFileWriterImpl(const oURIParts& _URIParts, bool _SupportAsyncWrites, bool* _pSuccess)
		: URIParts(_URIParts)
		, hFile(nullptr)
	{
		*_pSuccess = false;

		oSystemURIPartsToPath(ResolvedPath.Initialize(), _URIParts);
		
		ouro::filesystem::create_directories(path(*ResolvedPath).parent_path());

		// @tony:
		// FILE_SHARE_READ added here for a sole purpose: so a programmer can open
		// a log file and see stuff written so far while the log is still being 
		// appended to. If this causes problems, take this out immediately as no 
		// code per-sae relies on this behavior.
		hFile = CreateFile(*ResolvedPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0/* FILE_FLAG_OVERLAPPED*/, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			throw ouro::windows::error();
		else if (_SupportAsyncWrites)
		{
			if (!oDispatchQueueCreateGlobalIOCP("File writer dispatch queue", 16, &WriteQueue))
			{
				CloseHandle(hFile);
				throw std::invalid_argument("Could not create IOCP dispatch queue.");
			}
		}

		*_pSuccess = true;
	}

	~oFileWriterImpl()
	{
		if(WriteQueue)
			WriteQueue->Join();

		Close();
	}
	
	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override
	{
		oFileGetDesc(path(*ResolvedPath), _pDesc);
	}

	void DispatchWrite(const oSTREAM_WRITE& _Write, continuation_t _Continuation) threadsafe override
	{
		intrusive_ptr<threadsafe oFileWriterImpl> self(this);
		// Doing it this way because standard windows async file io is not 
		// GUARANTEED to be asynchronous. Windows can decided to do the read 
		// synchronously on its own, so FORCE async this way.
		WriteQueue->Dispatch([self, _Write, _Continuation]() mutable {
			bool success = self->Write(_Write);
			_Continuation(success, self.c_ptr(), _Write);
		});
	}

	bool Write(const oSTREAM_WRITE& _Write) threadsafe
	{
		// Need to lock since SetFilePointer/ReadFile are separate calls.
		lock_guard<mutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(std::errc::operation_canceled);
		}

		DWORD numBytesWritten;
		if (_Write.Range.Offset == oSTREAM_APPEND)
		{
			long offsetHigh = 0; // if you don't pass high, you can't seek to the end once file is larger than 4 gig
			SetFilePointer(hFile, 0, &offsetHigh, FILE_END);
		}
		else
		{
			long offsetLow, offsetHigh;
			oSetHighLowOffset(offsetLow, offsetHigh, _Write.Range.Offset);
			SetFilePointer(hFile, offsetLow, &offsetHigh, FILE_BEGIN);
		}

		oVB(WriteFile(hFile, _Write.pData, as_uint(_Write.Range.Size), &numBytesWritten, nullptr));

		if (numBytesWritten != _Write.Range.Size) // happens when trying to read past end of file, but ReadFile itself still succeeds
			oTHROW(io_error, "Failed to write data to disk.");

		return true;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		lock_guard<mutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oURIParts> URIParts;
	oInitOnce<path_string> ResolvedPath;
	oHandle hFile;
	intrusive_ptr<threadsafe oDispatchQueueGlobal> WriteQueue;
	mutex Mutex;
	oRefCount RefCount;
};

struct oFileSchemeHandlerImpl : oFileSchemeHandler
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oSchemeHandler, oFileSchemeHandler);

	int GetOrder() const threadsafe { return 1000; }
	const char* GetScheme() const threadsafe { return "file"; }
	bool GetDesc(const oURIParts& _URI, oSTREAM_DESC* _pDesc) threadsafe override;
	bool Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive = true) threadsafe override;
	bool Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination = false) threadsafe override;
	bool Delete(const oURIParts& _URIParts) threadsafe override;
	bool CreateStreamReader(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe override;
	bool CreateStreamReaderNonBuffered4K(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe override;
	bool CreateStreamWriter(const oURIParts& _URIParts, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe override;
	bool CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe override;

protected:
	oRefCount RefCount;
};

bool oFileSchemeHandlerCreate(threadsafe oFileSchemeHandler** _ppFileSchemeHandler)
{
	bool success = true;
	oCONSTRUCT(_ppFileSchemeHandler, oFileSchemeHandlerImpl());
	return success;
}

bool oFileSchemeHandlerImpl::GetDesc(const oURIParts& _URIParts, oSTREAM_DESC* _pDesc) threadsafe
{
	path_string Path;
	oSystemURIPartsToPath(Path, _URIParts);
	return oFileGetDesc(path(Path), _pDesc);
}

bool oFileSchemeHandlerImpl::Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive) threadsafe
{
	path_string S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);

	if (_Recursive)
		ouro::filesystem::copy_all(path(S), path(D), ouro::filesystem::copy_option::overwrite_if_exists);
	else
		ouro::filesystem::copy_file(path(S), path(D), ouro::filesystem::copy_option::overwrite_if_exists);

	return true;
}

bool oFileSchemeHandlerImpl::Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination) threadsafe
{
	path_string S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);
	ouro::filesystem::rename(path(S), path(D), _OverwriteDestination ? ouro::filesystem::copy_option::overwrite_if_exists : ouro::filesystem::copy_option::fail_if_exists);
	return true;
}

bool oFileSchemeHandlerImpl::Delete(const oURIParts& _URIParts) threadsafe
{
	path_string Path;
	oSystemURIPartsToPath(Path, _URIParts);
	ouro::filesystem::remove_all(path(Path));
	return true;
}

bool oFileSchemeHandlerImpl::CreateStreamReader(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppReader, oFileReaderImpl(_URIParts, false, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamReaderNonBuffered4K(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppReader, oFileReaderImpl(_URIParts, true, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamWriter(const oURIParts& _URIParts, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppWriter, oFileWriterImpl(_URIParts, _SupportAsyncWrites, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe
{
	bool success = false;
	oErrorSetLast(std::errc::function_not_supported, "use filesystem::monitor");
	return success;
}
