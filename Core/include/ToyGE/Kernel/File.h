#pragma once
#ifndef FILE_H
#define FILE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	enum FileSeekFlag
	{
		FILE_SEEK_BEGIN,
		FILE_SEEK_CUR,
		FILE_SEEK_END
	};

	enum FileOpenFlag : uint32_t
	{
		FILE_OPEN_READ = 1UL << 0UL,
		FILE_OPEN_WRITE = 1UL << 1UL,
		FILE_OPEN_APPEND = 1UL << 2UL
	};

	struct FileTime
	{
		uint64_t creationTime;
		uint64_t lastAccessTime;
		uint64_t lastWriteTime;
	};

	class TOYGE_CORE_API File
	{
	public:
		static bool Exist(const WString & path);

		static bool IsDirectory(const WString & path);

		static void MakeDirectory(const WString & path);

		static void MakeDirectoryRecur(const WString & path);

		static WString Search(const std::vector<WString> & paths, const WString & fileName);

		static WString GetCurProgramPath();

		static void SetCurrentPathToProgram();

		File(const WString & filePath, uint32_t openFlag);

		~File();

		void Release();

		size_t Size() const;

		size_t Pos() const;

		void Seek(fpos_t offset, FileSeekFlag flag);

		size_t Read(void *dst, size_t size);

		/*bool Write(const String & str);

		bool Write(const WString & str);*/

		bool Write(const void * src, size_t numBytes);

		void Flush();

		bool CanRead() const
		{
			return _fs && _fs->good() && _flag & FILE_OPEN_READ;
		}
		bool CanWrite() const
		{
			return _fs && _fs->good() && _flag & FILE_OPEN_WRITE;
		}

		bool Valid() const
		{
			return CanRead() || CanWrite();
		}

		operator bool() const
		{
			return Valid();
		}

		const WString & Path() const
		{
			return _path;
		}

		const WString & ParentPath() const
		{
			return _parentPath;
		}

		const WString & GetName() const
		{
			return _name;
		}

		const WString & GetExtensions() const
		{
			return _extensions;
		}

		FileTime GetTime() const;

		void MakeParentPathDirectory();

	private:
		WString _path;
		WString _parentPath;
		WString _name;
		WString _extensions;
		uint32_t _flag;
		std::fstream *_fs;

		void SplitPath();
	};

	class TOYGE_CORE_API FileReader : public Reader
	{
	public:
		FileReader(const Ptr<File> & file)
			: _file(file)
		{
		}

	protected:
		void DoReadBytes(void * dst, size_t numBytes) override
		{
			_file->Read(dst, numBytes);
		}

	private:
		Ptr<File> _file;
	};

	class TOYGE_CORE_API FileWriter : public Writer
	{
	public:
		FileWriter(const Ptr<File> & file)
			: _file(file)
		{
		}

	protected:
		void DoWriteBytes(const void * src, size_t numBytes) override
		{
			_file->Write(src, numBytes);
		}

	private:
		Ptr<File> _file;
	};
}

#endif