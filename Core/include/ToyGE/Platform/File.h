#pragma once
#ifndef FILE_H
#define FILE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\IOHelper.h"
#include <boost\noncopyable.hpp>

namespace ToyGE
{
	TOYGE_CORE_API String IdenticalPath(const String & path);

	TOYGE_CORE_API String ParentPath(const String & path);

	TOYGE_CORE_API void MakePathDirectorys(const String & path);

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

	class TOYGE_CORE_API File : public boost::noncopyable
	{
	public:
		File(const String & filePath, uint32_t openFlag);

		virtual ~File() = default;

		virtual void Close() = 0;

		virtual size_t Size() const = 0;

		virtual size_t PointerPos() const = 0;

		virtual void Seek(fpos_t offset, FileSeekFlag flag) = 0;

		virtual size_t Read(void *dst, size_t size) = 0;

		virtual size_t Write(const void * src, size_t numBytes) = 0;

		virtual void Flush() = 0;

		virtual FileTime GetTime() const = 0;

		const String & GetPath() const
		{
			return _path;
		}

		const String & GetName() const
		{
			return _name;
		}

		const String & GetExtension() const
		{
			return _extension;
		}

		bool IsDirectory() const
		{
			return _bDirectory;
		}

		bool IsValid() const
		{
			return _bValid;
		}

	protected:
		String _path;
		String _name;
		String _extension;
		uint32_t _flag;
		bool _bDirectory;
		bool _bValid;
	};

	class TOYGE_CORE_API FileReader : public Reader
	{
	public:
		FileReader(const Ptr<File> & file)
			: _file(file)
		{
		}

		CLASS_GET(File, Ptr<File>, _file);

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

		CLASS_GET(File, Ptr<File>, _file);

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