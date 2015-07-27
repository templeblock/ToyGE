#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\Assert.h"

#ifdef PLATFORM_WINDOWS
#include <Shlwapi.h>
#endif

namespace ToyGE
{
	bool File::Exist(const WString & path)
	{
#ifdef PLATFORM_WINDOWS
		return ::PathFileExistsW(path.c_str()) == TRUE;
#endif
	}

	bool File::IsDirectory(const WString & path)
	{
#ifdef PLATFORM_WINDOWS
		return ::PathIsDirectoryW(path.c_str()) == FILE_ATTRIBUTE_DIRECTORY;
#endif
	}

	void File::MakeDirectory(const WString & path)
	{
#ifdef PLATFORM_WINDOWS
		::CreateDirectoryW(path.c_str(), nullptr);
#endif
	}

	void File::MakeDirectoryRecur(const WString & path)
	{
		auto tmpPath = path;
		IdenticalPath(tmpPath);

		std::wstringstream wss;
		for (auto & ch : tmpPath)
		{
			wss << ch;
			if (ch == L'\\')
				MakeDirectory(wss.str());
		}

		if (tmpPath.back() != L'\\')
			MakeDirectory(tmpPath);
	}

	WString File::Search(const std::vector<WString> & paths, const WString & fileName)
	{
		if (File::Exist(fileName))
			return fileName;

		for (auto & basePath : paths)
		{
			auto absPath = basePath + fileName;
			if (File::Exist(absPath))
				return absPath;
		}

		return WString();
	}

	WString File::GetCurProgramPath()
	{
		WString path;
#ifdef PLATFORM_WINDOWS
		std::vector<wchar_t> buf;
		buf.resize(MAX_PATH);

		uint32_t cpyCount;
		while ((cpyCount = ::GetModuleFileNameW(nullptr, &buf[0], static_cast<uint32_t>(buf.size()))) == static_cast<uint32_t>(buf.size()))
			buf.resize(buf.size() * 2);

		path = &buf[0];
#endif

		return path;
	}

	void File::SetCurrentPathToProgram()
	{
#ifdef PLATFORM_WINDOWS
		auto programPath = GetCurProgramPath();
		auto curPath = programPath.substr(0, programPath.rfind('\\'));

		::SetCurrentDirectoryW(curPath.c_str());
#endif
	}

	File::File(const WString & filePath, uint32_t openFlag)
		: _path(filePath),
		_flag(openFlag),
		_fs(nullptr)
	{
		IdenticalPath(_path);

		if (!(openFlag & FILE_OPEN_READ || openFlag & FILE_OPEN_WRITE))
			return;

		std::ios::openmode mode = std::ios::binary;
		if (openFlag & FILE_OPEN_READ)
			mode |= std::ios::in;
		if (openFlag & FILE_OPEN_WRITE)
			mode |= std::ios::out;
		if (openFlag & FILE_OPEN_APPEND)
			mode |= std::ios::app;

		_fs = new std::fstream(_path, mode);

		SplitPath();
	}

	File::~File()
	{
		_fs->close();
		delete _fs;
	}

	size_t File::Size() const
	{
		size_t curPos = Pos();
		const_cast<File*>(this)->Seek(0, FILE_SEEK_END);
		size_t size = Pos();
		const_cast<File*>(this)->Seek(curPos, FILE_SEEK_BEGIN);
		return size;
	}

	size_t File::Pos() const
	{
		return static_cast<size_t>(_fs->tellg());
	}

	void File::Seek(fpos_t offset, FileSeekFlag flag)
	{
		switch (flag)
		{
		case ToyGE::FILE_SEEK_BEGIN:
			_fs->seekg(offset, std::ios::beg);
			break;

		case ToyGE::FILE_SEEK_CUR:
			_fs->seekg(offset, std::ios::cur);
			break;

		case ToyGE::FILE_SEEK_END:
			_fs->seekg(offset, std::ios::end);
			break;

		default:
			break;
		}
	}

	size_t File::Read(void *dst, size_t size)
	{
		ToyGE_ASSERT(dst);
		_fs->read(reinterpret_cast<char*>(dst), size);
		return static_cast<size_t>(_fs->gcount());
	}

	/*bool File::Write(const String & str)
	{
		_fs->write(str.c_str(), str.size());
		return _fs->good();
	}*/

	bool File::Write(const void * src, size_t numBytes)
	{
		_fs->write(reinterpret_cast<const char*>(src), numBytes);
		return _fs->good();
	}

	void File::Flush()
	{
		_fs->flush();
	}

	/*String File::ParentPath() const
	{
		return _path.substr(0, _path.rfind('\\') + 1);
	}*/

	void File::SplitPath()
	{
		auto rSpritFind = _path.rfind(L'\\');
		if (rSpritFind == std::wstring::npos)
			_parentPath = L"";
		else
			_parentPath = _path.substr(0, rSpritFind + 1);

		auto rDotFind = _path.rfind(L".");
		if (rDotFind == std::wstring::npos)
		{
			_name = _path.substr(_parentPath.size());
		}
		else
		{
			_name = _path.substr(_parentPath.size(), _path.rfind(L".") - _parentPath.size());
			_extensions = _path.substr(rDotFind + 1);
		}
	}

	FileTime File::GetTime() const
	{
		FileTime ft;
		memset(&ft, 0, sizeof(ft));
#ifdef PLATFORM_WINDOWS
		auto hFile = ::CreateFileW(_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::GetFileTime(
				hFile,
				reinterpret_cast<LPFILETIME>(&ft.creationTime),
				reinterpret_cast<LPFILETIME>(&ft.lastAccessTime),
				reinterpret_cast<LPFILETIME>(&ft.lastWriteTime));
			::CloseHandle(hFile);
		}
#endif
		return ft;
	}

	void File::MakeParentPathDirectory()
	{
		if (IsDirectory(this->ParentPath()))
			return;

		MakeDirectoryRecur(this->ParentPath());
	}
	
}