// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/filesystem.h>
#include <oCore/process.h>
#include <oCore/system.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_iocp.h>
#include <oBase/date.h>
#include <oBase/macros.h>
#include <oString/string.h>
#include <oMemory/memory.h>

#include <io.h>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>

using namespace std;

namespace ouro {

const char* as_string(const filesystem::file_type::value& _Type)
{
	switch (_Type)
	{
		case filesystem::file_type::block_file: return "block_file";
		case filesystem::file_type::character_file: return "character_file";
		case filesystem::file_type::directory_file: return "directory_file";
		case filesystem::file_type::fifo_file: return "fifo_file";
		case filesystem::file_type::file_not_found: return "file_not_found";
		case filesystem::file_type::regular_file: return "regular_file";
		case filesystem::file_type::socket_file: return "socket_file";
		case filesystem::file_type::status_unknown: return "status_unknown";
		case filesystem::file_type::symlink_file: return "symlink_file";
		case filesystem::file_type::type_unknown: return "type_unknown";
		case filesystem::file_type::read_only_directory_file: return "read_only_directory_file";
		case filesystem::file_type::read_only_file: return "read_only_file";
		default: break;
	}
	return "?";
}

}

#define oFSTHROW0(_ErrCode) throw filesystem_error(make_error_code(errc::_ErrCode))
#define oFSTHROW01(_ErrCode, _Path1) throw filesystem_error(_Path1, make_error_code(errc::_ErrCode))
#define oFSTHROW02(_ErrCode, _Path1, _Path2) throw filesystem_error(_Path1, _Path2, make_error_code(errc::_ErrCode))

#define oFSTHROW(_ErrCode, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, make_error_code(errc::_ErrCode)); \
} while (false)

#define oFSTHROW1(_ErrCode, _Path1, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, _Path1, make_error_code(errc::_ErrCode)); \
} while (false)

#define oFSTHROW2(_ErrCode, _Path1, _Path2, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, _Path1, _Path2, make_error_code(errc::_ErrCode)); \
} while (false)

#define oFSTHROWLAST() throw windows::error()
#define oFSTHROWLAST1(_Path1) throw windows::error()
#define oFSTHROWLAST2(_Path1, _Path2) throw windows::error()

#define oFSTHROW_FOPEN(err, _Path) do \
{	char strerr[256]; \
	strerror_s(strerr, err); \
	throw filesystem_error(strerr, _Path, make_error_code((errc::errc)err)); \
} while (false)

#define oFSTHROW_FOPEN0() do \
{	errno_t err = 0; \
	_get_errno(&err); \
	char strerr[256]; \
	strerror_s(strerr, err); \
	throw filesystem_error(strerr, make_error_code((errc::errc)err)); \
} while (false)

namespace ouro {
	namespace filesystem {
		namespace detail {

class filesystem_category_impl : public error_category
{
public:
	const char* name() const override { return "filesystem"; }
	string message(int _ErrCode) const override
	{
		return generic_category().message(_ErrCode);
	}
};

		} // namespace detail

const error_category& filesystem_category()
{
	static detail::filesystem_category_impl sSingleton;
	return sSingleton;
}

static bool is_dot(const char* _Filename)
{
	return (!strcmp("..", _Filename) || !strcmp(".", _Filename));
}

path app_path(bool _IncludeFilename)
{
	char Path[MAX_PATH];
	DWORD len = GetModuleFileNameA(GetModuleHandle(nullptr), Path, oCOUNTOF(Path));
	if (!len)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			oTHROW0(no_buffer_space);
		oTHROW0(operation_not_permitted);
	}

	if (!_IncludeFilename)
	{
		char* p = Path + len - 1;
		while (*p != '\\' && p >= Path)
			p--;
		if (p < Path)
			oTHROW0(operation_not_permitted);
		*(++p) = '\0';
	}

	return Path;
}

path temp_path(bool _IncludeFilename)
{
	char Path[MAX_PATH+1];
	if (!GetTempPathA(oCOUNTOF(Path), Path))
		oTHROW0(operation_not_permitted);
	oVB(!_IncludeFilename || !GetTempFileNameA(Path, "tmp", 0, Path));
	return Path;
}

path log_path(bool _IncludeFilename, const char* _ExeSuffix)
{
	path Path = app_path(true);
	path Basename = Path.basename();
	Path.replace_filename();
	Path /= "Logs/";

	if (_IncludeFilename)
	{
		time_t theTime;
		time(&theTime);
		tm t;
		localtime_s(&t, &theTime);

		sstring StrTime;
		::strftime(StrTime, StrTime.capacity(), "%Y%m%d%H%M%S", &t);

		path_string StrLogFilename;
		snprintf(StrLogFilename, "%s-%s-%u%s%s.txt"
			, Basename.c_str()
			, StrTime.c_str()
			, this_process::get_id()
			, _ExeSuffix ? "-" : ""
			, _ExeSuffix ? _ExeSuffix : "");

		Path.replace_filename(StrLogFilename);
	}

	return Path;
}

path desktop_path()
{
	char Path[MAX_PATH];
	if (!SHGetSpecialFolderPathA(nullptr, Path, CSIDL_DESKTOPDIRECTORY/*CSIDL_COMMON_DESKTOPDIRECTORY*/, FALSE))
		oTHROW0(operation_not_permitted);
	if (strlcat(Path, "\\") >= oCOUNTOF(Path))
		oTHROW0(no_buffer_space);
	return Path;
}

path system_path()
{
	char Path[MAX_PATH];
	UINT len = GetSystemDirectoryA(Path, oCOUNTOF(Path));
	if (!len)
		oTHROW0(operation_not_permitted);
	if (len > (MAX_PATH-2))
		oTHROW0(no_buffer_space);
	Path[len] = '\\';
	Path[len+1] = '\0';
	return Path;
}

path os_path()
{
	char Path[MAX_PATH];
	UINT len = GetWindowsDirectoryA(Path, oCOUNTOF(Path));
	if (!len)
		oTHROW0(operation_not_permitted);
	if (len > (MAX_PATH-2))
		oTHROW0(no_buffer_space);
	Path[len] = '\\';
	Path[len+1] = '\0';
	return Path;
}

path dev_path()
{
	path Root = app_path();
	while (1)
	{
		path leaf = Root.filename();
		if (!_stricmp("bin", leaf))
			return Root.remove_filename();
		Root.remove_filename();
		if (Root.empty())
			oTHROW0(no_such_file_or_directory );
	} while (!Root.empty());
}

path data_path()
{
	path Data;
	try { Data = dev_path(); }
	catch (filesystem_error&) { Data = app_path(); }
	Data /= "data/";

	return Data;
}

path current_path()
{
	char Path[MAX_PATH];
	if (!GetCurrentDirectoryA(oCOUNTOF(Path), Path))
		oTHROW0(operation_not_permitted);
	
	return Path;
}

void current_path(const path& _Path)
{
	oVB_MSG(SetCurrentDirectory(_Path), "Path: %s", _Path.c_str());
		oFSTHROWLAST1(_Path);
}

path root_path(const char* _RootName)
{
	if (!_stricmp("data", _RootName)) return data_path();
	if (!_stricmp("app", _RootName)) return app_path();
	if (!_stricmp("current", _RootName)) return current_path();
	if (!_stricmp("desktop", _RootName)) return desktop_path();
	if (!_stricmp("temp", _RootName)) return temp_path();
	if (!_stricmp("dev", _RootName)) return dev_path();
	if (!_stricmp("system", _RootName)) return system_path();
	if (!_stricmp("os", _RootName)) return os_path();
	if (!_stricmp("log", _RootName)) return log_path();
	return "";
}

path resolve(const path& _RelativePath)
{
	#define APPEND() \
		AbsolutePath /= _RelativePath; \
		if (exists(AbsolutePath)) return AbsolutePath;

	path AbsolutePath = app_path();
	APPEND();
	AbsolutePath = current_path();
	APPEND();
	AbsolutePath = system_path();
	APPEND();
	AbsolutePath = os_path();
	APPEND();
	AbsolutePath = desktop_path();
	APPEND();
	AbsolutePath = data_path();
	APPEND();
	AbsolutePath = temp_path();
	APPEND();

	char PATH[oKB(4)];
	if (system::getenv(PATH, "PATH"))
	{
		char result[512];
		path CWD = current_path();
		if (search_path(result, PATH, _RelativePath.c_str(), CWD.c_str(), [&](const char* _Path)->bool { return exists(_Path); }))
			return result;
	}

	oFSTHROW01(no_such_file_or_directory, _RelativePath);
}

static file_status status_internal(DWORD _dwFileAttributes)
{
	if (_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		return file_status(file_type::file_not_found);

	if (_dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (_dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			return file_status(file_type::read_only_directory_file);
		return file_status(file_type::directory_file);
	}
	
	if (_dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		return file_status(file_type::read_only_file);
	
	// todo: add determination if a symlink_file
	// what are block/character file-s on Windows?

	return file_status(file_type::regular_file);
}

file_status status(const path& _Path)
{
	return status_internal(GetFileAttributesA(_Path));
}

bool exists(const path& _Path)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(_Path);
}

void read_only(const path& _Path, bool _ReadOnly)
{
	DWORD dwFileAttributes = GetFileAttributesA(_Path);
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		oFSTHROW01(no_such_file_or_directory, _Path);

	if (_ReadOnly) dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
	else dwFileAttributes &=~ FILE_ATTRIBUTE_READONLY;

	if (!SetFileAttributesA(_Path, dwFileAttributes))
		oFSTHROWLAST1(_Path);
}

template<typename WIN32_TYPE> static unsigned long long file_size_internal(const WIN32_TYPE& _FileData)
{
	return (_FileData.nFileSizeHigh * (static_cast<unsigned long long>(MAXDWORD) + 1)) 
		+ _FileData.nFileSizeLow;
}

unsigned long long file_size(const path& _Path)
{
	WIN32_FILE_ATTRIBUTE_DATA fd;
	if (!GetFileAttributesExA(_Path, GetFileExInfoStandard, &fd))
		oFSTHROWLAST1(_Path);
	return file_size_internal(fd);
}

time_t last_write_time(const path& _Path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (0 == GetFileAttributesExA(_Path, GetFileExInfoStandard, &fad))
		oFSTHROWLAST1(_Path);
	return date_cast<time_t>(fad.ftLastWriteTime);
}

void last_write_time(const path& _Path, time_t _Time)
{
	HANDLE hFile = CreateFileA(_Path, FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		oFSTHROWLAST1(_Path);
	finally CloseFile([&] { CloseHandle(hFile); });
	FILETIME time = date_cast<FILETIME>(_Time);
	if (!SetFileTime(hFile, nullptr, nullptr, &time))
		oFSTHROWLAST1(_Path);
}

bool remove_filename(const path& _Path)
{
	return !!DeleteFileA(_Path);
}

bool remove_directory(const path& _Path)
{
	int Retried = 0;
Retry:

	if (!RemoveDirectory(_Path))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_DIR_NOT_EMPTY:
			{
				// if a file explorer is open, this may be a false positive
				this_thread::sleep_for(chrono::milliseconds(100));
				if (Retried > 1)
					oFSTHROW01(directory_not_empty, _Path);
				Retried++;
				goto Retry;
			}
			case ERROR_SHARING_VIOLATION: oFSTHROW01(device_or_resource_busy, _Path);
			default: break;
		}
		oFSTHROWLAST1(_Path);
	}

	return true;
}

unsigned int remove_all(const path& _Path)
{
	if (is_directory(_Path))
	{
		path wildcard(_Path);
		wildcard /= "*";
		unsigned int removed = 0;
		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			if (!is_directory(_Status))
				removed += remove_filename(_FullPath) ? 1 : 0;
			return true;
		});

		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			if (is_directory(_Status))
				removed += remove_all(_FullPath);
			return true;
		});

		removed += remove_directory(_Path) ? 1 : 0;
		return removed;
	}

	return remove_filename(_Path) ? 1 : 0;
}

void copy_file(const path& _From, const path& _To, copy_option::value _Option)
{
	if (!CopyFileA(_From, _To, _Option == copy_option::fail_if_exists))
		oFSTHROWLAST2(_From, _To);
}

unsigned int copy_all(const path& _From, const path& _To, copy_option::value _Option)
{
	if (is_directory(_From))
	{
		unsigned int copied = 0;
		file_status ToStatus = status(_To);

		if (!exists(ToStatus))
			create_directory(_To);

		else if (!is_directory(ToStatus))
			oFSTHROW02(is_a_directory, _From, _To);

		path wildcard(_From);
		wildcard /= "*";
		path FullTo(_To);
		FullTo /= "*"; // put a "filename" so it can be replaced below
		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			FullTo.replace_filename(_FullPath.filename());

			if (is_directory(_Status))
			{
				copied += copy_all(_FullPath, FullTo, _Option);
			}

			else
			{
				copy_file(_FullPath, FullTo, _Option);
				copied++;
			}
			
			return true;
		});

		return copied;
	}
	copy_file(_From, _To, _Option);
	return 1;
}

void rename(const path& _From, const path& _To, copy_option::value _Option)
{
	if (!exists(_From))
		oFSTHROW01(no_such_file_or_directory, _From);

	if (_Option == copy_option::overwrite_if_exists && exists(_To))
		remove_all(_To);

	MoveFileA(_From, _To);
}

bool create_directory(const path& _Path)
{
	if (!CreateDirectoryA(_Path, nullptr))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_ALREADY_EXISTS: return false;
			case ERROR_PATH_NOT_FOUND: oFSTHROW01(no_such_file_or_directory, _Path); break;
			default: oFSTHROWLAST();
		}
	}

	return true;
}

static bool create_directories_internal(const path& _Path)
{
	if (_Path.empty())
		return false;

	if (!CreateDirectoryA(_Path, nullptr))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_ALREADY_EXISTS: return false;
			case ERROR_PATH_NOT_FOUND:
			{
				path parent = _Path.parent_path();

				if (!create_directories_internal(parent))
					return false; // pass thru error message

				// Now try again
				return create_directory(_Path);
			}
			default: oFSTHROWLAST1(_Path);
		}
	}

	return true;
}

bool create_directories(const path& _Path)
{
	return create_directories_internal(_Path);
}

// This returns information for the volume containing _Path
space_info space(const path& _Path)
{
	space_info s;
	if (!GetDiskFreeSpaceEx(_Path, (PULARGE_INTEGER)&s.available, (PULARGE_INTEGER)&s.capacity, (PULARGE_INTEGER)&s.free))
		oFSTHROWLAST1(_Path);
	return s;
}

// Enumerates all entries matching _WildcardPath
void enumerate(const path& _WildcardPath
	, const function<bool(const path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(_WildcardPath, &fd);
	finally CloseSearch([&](){ if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind); });

	if (hFind != INVALID_HANDLE_VALUE)
	{
		path resolved(_WildcardPath);

		while (true)
		{
			if (!is_dot(fd.cFileName))
			{
				file_status status = status_internal(fd.dwFileAttributes);
				resolved.replace_filename(fd.cFileName);
				if (!_Enumerator(resolved, status, file_size_internal(fd)))
					break;
			}

			if (!FindNextFile(hFind, &fd))
				break;
		}
	}
}

void* map(const path& _Path
	, map_option::value _MapOption
	, unsigned long long _Offset
	, unsigned long long _Size)
{
	HANDLE hFile = CreateFileA(_Path, _MapOption == map_option::binary_read ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE), 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		oFSTHROWLAST();
	finally CloseFile([&] { CloseHandle(hFile); });

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	byte_swizzle64 alignedOffset;
	alignedOffset.as_ullong = byte_align_down(_Offset, si.dwAllocationGranularity);
	unsigned long long offsetPadding = _Offset - alignedOffset.as_ullong;
	unsigned long long alignedSize = _Size + offsetPadding;

	DWORD fProtect = _MapOption == map_option::binary_read ? PAGE_READONLY : PAGE_READWRITE;
	HANDLE hMapped = CreateFileMapping(hFile, nullptr, fProtect, 0, 0, nullptr);
	if (!hMapped)
		oFSTHROWLAST();
	finally CloseMapped([&] { CloseHandle(hMapped); });

	void* p = MapViewOfFile(hMapped, fProtect == PAGE_READONLY ? FILE_MAP_READ : FILE_MAP_WRITE, alignedOffset.as_uint[1], alignedOffset.as_uint[0], as_type<SIZE_T>(alignedSize));
	if (!p)
		oFSTHROWLAST();

	// Exit with a ref count of 1 on the underlying HANDLE, held by MapViewOfFile
	// so the file is fully closed when unmap is called. Right now the count is at 
	// 3, but the finallies will remove 2.
	return byte_add(p, as_size_t(offsetPadding));
}

void unmap(void* _MappedPointer)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	void* p = byte_align_down(_MappedPointer, si.dwAllocationGranularity);
	if (!UnmapViewOfFile(p))
		oFSTHROWLAST();
}

native_file_handle get_native(file_handle _hFile)
{
	return (native_file_handle)_get_osfhandle(_fileno(_hFile));
}

file_handle open(const path& _Path, open_option::value _OpenOption)
{
	static char* opt = "rwa";

	char open[3];
	open[0] = opt[_OpenOption % 3];
	open[1] = _OpenOption >= open_option::text_read ? 't' : 'b';
	open[2] = '\0';

	_set_errno(0);
	file_handle hfile = (file_handle)_fsopen(_Path, open, _SH_DENYNO);
	if (!hfile)
	{
		errno_t err = 0;
		_get_errno(&err);
		oFSTHROW_FOPEN(err, _Path);
	}
	return hfile;
}

void close(file_handle _hFile)
{
	if (!_hFile)
		throw filesystem_error(make_error_code(errc::invalid_argument));
	if (0 != fclose((FILE*)_hFile))
		oFSTHROW_FOPEN0();
}

bool at_end(file_handle _hFile)
{
	return !!feof((FILE*)_hFile);
}

path get_path(file_handle _hFile)
{
	#if WINVER < 0x0600 // NTDDI_VISTA
		#error this function is implemented only for Windows Vista and later.
	#endif
	HANDLE f = get_native(_hFile);
	path_string str;
	oVB(GetFinalPathNameByHandleA(f, str.c_str(), static_cast<DWORD>(str.capacity()), 0));
	return path(str);
}

void last_write_time(file_handle _hFile, time_t _Time)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_hFile));
	if (hFile == INVALID_HANDLE_VALUE)
		oTHROW0(no_such_file_or_directory);
	FILETIME time = date_cast<FILETIME>(_Time);
	oVB(SetFileTime(hFile, 0, 0, &time));
}

void seek(file_handle _hFile, long long _Offset, seek_origin::value _Origin)
{ 
	if (-1 == _fseeki64((FILE*)_hFile, _Offset, (int)_Origin))
		oFSTHROW_FOPEN0();
}

unsigned long long file_size(file_handle _hFile)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_hFile));
	if (hFile == INVALID_HANDLE_VALUE)
		oTHROW0(no_such_file_or_directory);
	
	#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
		FILE_STANDARD_INFO fsi;
		oVB(GetFileInformationByHandleEx(hFile, FileStandardInfo, &fsi, sizeof(fsi)));
		return fsi.EndOfFile.QuadPart;
	#else
		LARGE_INTEGER fsize;
		oVB(GetFileSizeEx(hFile, &fsize));
		return fsize.QuadPart;
	#endif
}

unsigned long long tell(file_handle _hFile)
{
	unsigned long long offset = _ftelli64((FILE*)_hFile);
	if (offset == 1L)
		oFSTHROW_FOPEN0();
	return offset;
}

unsigned long long read(file_handle _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize)
{
	const size_t ReadSize = as_size_t(_ReadSize);
	size_t bytesRead = fread_s(_pDestination, as_size_t(_SizeofDestination), 1, ReadSize, (FILE*)_hFile);
	if (ReadSize != bytesRead && !at_end(_hFile))
		oFSTHROW_FOPEN0();
	return bytesRead;
}

unsigned long long write(file_handle _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush)
{
	size_t bytesWritten = fwrite(_pSource, 1, static_cast<size_t>(_WriteSize), (FILE*)_hFile);
	if (as_size_t(_WriteSize) != bytesWritten)
		oFSTHROW_FOPEN0();
	if (_Flush)
		fflush((FILE*)_hFile);
	return bytesWritten;
}

void save(const path& _Path, const void* _pSource, size_t _SizeofSource, save_option::value _SaveOption)
{
	open_option::value OpenOption = open_option::binary_read;
	switch (_SaveOption)
	{
		case save_option::text_write: OpenOption = open_option::text_write; break;
		case save_option::text_append: OpenOption = open_option::text_append; break;
		case save_option::binary_write: OpenOption = open_option::binary_write; break;
		case save_option::binary_append: OpenOption = open_option::binary_append; break;
		default: oASSUME(0);
	}
	
	create_directories(_Path.parent_path());
	file_handle f = open(_Path, OpenOption);
	finally CloseFile([&] { close(f); });
	write(f, _pSource, _SizeofSource);
}

void delete_buffer(char* _pBuffer)
{
	delete [] _pBuffer;
}

scoped_allocation load(const path& _Path, load_option::value _LoadOption, const allocator& _Allocator)
{
	unsigned long long FileSize = as_size_t(file_size(_Path));
	// in case we need a UTF32 nul terminator
	size_t AllocSize = as_size_t(FileSize + (_LoadOption == load_option::text_read ? 4 : 0));
	scoped_allocation p = _Allocator.scoped_allocate(AllocSize);

	// put enough nul terminators for a UTF32 or 16 or 8
	if (_LoadOption == load_option::text_read)
		*(int*)&(((char*)p)[FileSize]) = 0;

	{
		file_handle f = open(_Path, _LoadOption == load_option::text_read ? open_option::text_read : open_option::binary_read);
		finally CloseFile([&] { close(f); });
		if (FileSize != read(f, p, AllocSize, FileSize) && _LoadOption == load_option::binary_read)
			oTHROW(io_error, "read failed: %s", _Path.c_str());
	}

	// record honest size (tools like FXC crash if any larger size is given)
	size_t HonestSize = as_size_t(FileSize);

	if (_LoadOption == load_option::text_read)
	{
		utf_type type = utfcmp(p, __min(HonestSize, 512));
		switch (type)
		{
			case utf_type::utf32be: case utf_type::utf32le: HonestSize += 4; break;
			case utf_type::utf16be: case utf_type::utf16le: HonestSize += 2; break;
			case utf_type::ascii: HonestSize += 1; break;
		}
	}

	return scoped_allocation(p.release(), HonestSize, p.get_deallocate());
}

struct iocp_file
{
	iocp_file(const path& _Path, const allocator& _Allocator, const std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)>& _OnComplete)
		: overlapped(nullptr)
		, handle(INVALID_HANDLE_VALUE)
		, file_size(0)
		, file_path(_Path)
		, allocator(_Allocator)
		, error_code(0)
		, open_type(open_read)
		, completion(_OnComplete)
	{}

	enum open
	{
		open_read,
		open_read_unbuffered,
		open_read_text,
		open_read_unbuffered_text,
		open_write,
	};

	scoped_allocation buffer;
	OVERLAPPED* overlapped;
	HANDLE handle;
	unsigned int file_size;
	path file_path;
	allocator allocator;
	long error_code;
	open open_type;
	std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)> completion;
};

static void iocp_close(iocp_file* f, unsigned long long _Unused = 0)
{
	if (!f->error_code && (f->open_type == iocp_file::open_read_text || f->open_type == iocp_file::open_read_unbuffered_text))
	{
		char* p = (char*)f->buffer;
		*(int*)&(p[f->file_size]) = 0;
		// the above implies support for UTF32, but this replace does not
		replace(p, f->file_size + 4, p, "\r\n", "\n");
	}

	if (f->completion)
	{
		if (!f->error_code)
		{
			DWORD dwNumBytes = 0;
			if (!GetOverlappedResult(f->handle, f->overlapped, &dwNumBytes, FALSE))
				f->error_code = GetLastError();
		}

		if (f->error_code)
		{
			windows::error winerr(f->error_code);
			oASSERT(f->file_size == f->buffer.size(), "size mismatch");
			f->completion(std::ref(f->file_path), std::ref(f->buffer), &winerr);
		}
		else
			f->completion(std::ref(f->file_path), std::ref(f->buffer), nullptr);
	}

	if (f->handle != INVALID_HANDLE_VALUE)
	{
		f->error_code = CloseHandle(f->handle) ? 0 : GetLastError();
		f->handle = INVALID_HANDLE_VALUE;
	}

	f->file_size = 0;
	f->completion = nullptr;
	if (f->overlapped)
		windows::iocp::disassociate(f->overlapped);
	delete f;
}

static void iocp_open(iocp_file* f, iocp_file::open _Open)
{
	f->overlapped = nullptr;
	f->open_type = _Open;
	f->handle = CreateFile(f->file_path
		, _Open == iocp_file::open_write ? GENERIC_WRITE : GENERIC_READ
		, FILE_SHARE_READ
		, nullptr
		, _Open == iocp_file::open_write ? CREATE_ALWAYS : OPEN_EXISTING
		, FILE_FLAG_OVERLAPPED | ((f->open_type == iocp_file::open_read_unbuffered || f->open_type == iocp_file::open_read_unbuffered_text) ? FILE_FLAG_NO_BUFFERING : 0)
		, nullptr);
	if (f->handle != INVALID_HANDLE_VALUE)
	{
		f->error_code = 0;
		DWORD hi = 0;
		f->file_size = GetFileSize(f->handle, &hi);
		if (sizeof(f->file_size) > sizeof(DWORD))
			f->file_size |= ((unsigned long long)hi << 32);
	}
	else
	{
		f->error_code = GetLastError();
		iocp_close(f);
	}
}

static unsigned int iocp_allocation_size(const iocp_file* f)
{
	unsigned int size = f->file_size;
	if (f->open_type == iocp_file::open_read_text || f->open_type == iocp_file::open_read_unbuffered_text)
		size += 4; // worst-case nul terminator for Unicode 32

	if (f->open_type == iocp_file::open_read_unbuffered || f->open_type == iocp_file::open_read_unbuffered_text)
		size = byte_align(size, 4096);

	return size;
}

static unsigned int iocp_io_size(const iocp_file* f)
{
	unsigned int size = f->file_size;

	if (f->open_type == iocp_file::open_read_unbuffered || f->open_type == iocp_file::open_read_unbuffered_text)
		size = byte_align(size, 4096);

	return size;
}

static void iocp_read(iocp_file* f, unsigned int _Offset = 0, unsigned int _ReadSize = invalid)
{
	f->overlapped = windows::iocp::associate(f->handle, iocp_close, f);
	f->overlapped->Offset = (DWORD)_Offset;
	f->overlapped->OffsetHigh = 0;
	if (_ReadSize == invalid)
		_ReadSize = iocp_io_size(f);
	if (!ReadFile(f->handle, f->buffer, as_uint(_ReadSize), nullptr, f->overlapped))
	{
		int errc = GetLastError();
		if (errc != ERROR_IO_PENDING)
		{
			iocp_close(f);
			f->error_code = errc;
		}
	}
}

static void iocp_write(iocp_file* f, unsigned int _Offset = invalid, unsigned int _WriteSize = invalid)
{
	f->overlapped = windows::iocp::associate(f->handle, iocp_close, f);
	f->overlapped->Offset = (DWORD)_Offset;
	f->overlapped->OffsetHigh = 0;

	if (_WriteSize == invalid)
		_WriteSize = as_uint(f->buffer.size());
	
	if (!WriteFile(f->handle, f->buffer, _WriteSize, nullptr, f->overlapped))
	{
		int errc = GetLastError();
		if (errc != ERROR_IO_PENDING)
		{
			iocp_close(f);
			f->error_code = errc;
		}
	}
}

static void iocp_open_and_read_shared(iocp_file::open _OpenType, void* _pContext, unsigned long long _Unused = 0)
{
	iocp_file* f = (iocp_file*)_pContext;

	iocp_open(f, _OpenType);
	if (!f->error_code)
	{
		f->buffer = std::move(f->allocator.scoped_allocate(iocp_allocation_size(f)));
		if (f->buffer)
			iocp_read(f);			
		else
			iocp_close(f);
	}
}

static void iocp_open_and_read(void* _pContext, unsigned long long _Unused = 0)
{
	iocp_open_and_read_shared(iocp_file::open_read, _pContext, _Unused);
}

static void iocp_open_and_read_text(void* _pContext, unsigned long long _Unused = 0)
{
	iocp_open_and_read_shared(iocp_file::open_read_text, _pContext, _Unused);
}

static void iocp_open_and_write(void* _pContext, unsigned long long _Unused = 0)
{
	iocp_file* f = (iocp_file*)_pContext;
	if (f->buffer)
	{
		iocp_open(f, iocp_file::open_write);
		if (!f->error_code)
			iocp_write(f, 0);
	}
}

static void iocp_open_and_append(void* _pContext, unsigned long long _Unused = 0)
{
	iocp_file* f = (iocp_file*)_pContext;
	if (f->buffer)
	{
		iocp_open(f, iocp_file::open_write);
		if (!f->error_code)
			iocp_write(f);
	}
}

void load_async(const path& _Path
								, const std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)>& _OnComplete
								, load_option::value _LoadOption
								, const allocator& _Allocator)
{
	iocp_file* r = new iocp_file(_Path, _Allocator, _OnComplete);
	windows::iocp::post(_LoadOption == load_option::binary_read ? iocp_open_and_read : iocp_open_and_read_text, r);
}

void save_async(const path& _Path
								, scoped_allocation&& _Buffer
								, const std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)>& _OnComplete
								, save_option::value _SaveOption)
{
	if (_SaveOption != save_option::binary_write && _SaveOption != save_option::binary_append)
		oTHROW_INVARG("only binary_write and binary_append is currently supported");

	iocp_file* r = new iocp_file(_Path, allocator(), _OnComplete);
	r->buffer = std::move(_Buffer);
	windows::iocp::post(_SaveOption == save_option::binary_write ? iocp_open_and_write : iocp_open_and_append, r);
}

void wait()
{
	windows::iocp::wait();
}

bool wait_for(unsigned int _TimeoutMS)
{
	return windows::iocp::wait_for(_TimeoutMS);
}

bool joinable()
{
	return windows::iocp::joinable();
}

void join()
{
	windows::iocp::join();
}



	} // namespace filesystem
}
