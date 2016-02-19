#include "ToyGE\Platform\Windows\WindowsFile.h"
#include "ToyGE\Kernel\Core.h"

namespace ToyGE
{
	WindowsFile::WindowsFile(const String & filePath, uint32_t openFlag)
		: File(filePath, openFlag)
	{
		_u16Path = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>(_path);

		uint32_t accessFlag = 0;
		uint32_t createFlag = 0;
		uint32_t shareFlag = FILE_SHARE_READ;

		if (_flag & FILE_OPEN_READ)
		{
			accessFlag |= GENERIC_READ;
			createFlag = OPEN_EXISTING;
		}
		if (_flag & FILE_OPEN_WRITE)
		{
			accessFlag |= GENERIC_WRITE;
			createFlag = CREATE_ALWAYS;
		}
		if (_flag & FILE_OPEN_APPEND)
		{
			accessFlag |= GENERIC_WRITE;
			createFlag = TRUNCATE_EXISTING;
		}

		_fileHandle = ::CreateFileW(reinterpret_cast<const wchar_t*>(_u16Path.c_str()), accessFlag, shareFlag, 0, createFlag, FILE_ATTRIBUTE_NORMAL, 0);
		if (_fileHandle == INVALID_HANDLE_VALUE)
		{
			_bValid = false;
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				ToyGE_LOG(LT_WARNING, "File not found! %s", filePath.c_str());
			else if (GetLastError() == ERROR_SHARING_VIOLATION)
				ToyGE_LOG(LT_WARNING, "File share violation! %s", filePath.c_str());
		}
		else
			_bValid = true;
	}

	void WindowsFile::Close()
	{
		::CloseHandle(_fileHandle);
	}

	size_t WindowsFile::Size() const
	{
		size_t size = 0;
		LARGE_INTEGER lint;
		if (IsValid())
		{
			if (::GetFileSizeEx(_fileHandle, &lint))
				size = static_cast<size_t>( lint.QuadPart );
		}
		return size;
	}

	size_t WindowsFile::PointerPos() const
	{
		if (IsValid())
		{
			LARGE_INTEGER lint;
			lint.QuadPart = 0;
			::SetFilePointerEx(_fileHandle, lint, &lint, FILE_CURRENT);
			return static_cast<size_t>(lint.QuadPart);
		}
		return 0;
	}

	void WindowsFile::Seek(fpos_t offset, FileSeekFlag flag)
	{
		if (IsValid())
		{
			uint32_t seekFlag = 0;
			if (flag == FileSeekFlag::FILE_SEEK_BEGIN)
				seekFlag = FILE_BEGIN;
			else if(flag == FileSeekFlag::FILE_SEEK_CUR)
				seekFlag = FILE_CURRENT;
			else
				seekFlag = FILE_END;

			LARGE_INTEGER lint;
			lint.QuadPart = offset;
			::SetFilePointerEx(_fileHandle, lint, nullptr, seekFlag);
		}
	}

	size_t WindowsFile::Read(void *dst, size_t size)
	{
		if (IsValid())
		{
			DWORD numBytesRead;
			if (::ReadFile(_fileHandle, dst, (DWORD)size, &numBytesRead, nullptr))
				return static_cast<size_t>(numBytesRead);
		}
		return 0;
	}

	size_t WindowsFile::Write(const void * src, size_t numBytes)
	{
		if (IsValid())
		{
			DWORD numBytesWritten;
			if( ::WriteFile(_fileHandle, src, (DWORD)numBytes, &numBytesWritten, nullptr))
				return static_cast<size_t>(numBytesWritten);
		}
		return 0;
	}

	void WindowsFile::Flush()
	{
		if (IsValid())
			::FlushFileBuffers(_fileHandle);
	}

	FileTime WindowsFile::GetTime() const
	{
		FileTime ft = { 0 };
		if (IsValid())
		{
			if (::GetFileTime(
				_fileHandle,
				reinterpret_cast<LPFILETIME>(&ft.creationTime),
				reinterpret_cast<LPFILETIME>(&ft.lastAccessTime),
				reinterpret_cast<LPFILETIME>(&ft.lastWriteTime)))
				return ft;
			else
				return { 0 };
		}
		return ft;
	}
}