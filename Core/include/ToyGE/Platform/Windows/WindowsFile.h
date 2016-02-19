#pragma once
#ifndef WINDOWSFILE_H
#define WINDOWSFILE_H

#include "ToyGE\Platform\File.h"

namespace ToyGE
{
	class WindowsFile : public File
	{
	public:
		WindowsFile(const String & filePath, uint32_t openFlag);

		virtual ~WindowsFile()
		{
			Close();
		}

		virtual void Close() override;

		virtual size_t Size() const override;

		virtual size_t PointerPos() const override;

		virtual void Seek(fpos_t offset, FileSeekFlag flag) override;

		virtual size_t Read(void *dst, size_t size) override;

		virtual size_t Write(const void * src, size_t numBytes) override;

		virtual void Flush() override;

		virtual FileTime GetTime() const override;

	private:
		HANDLE _fileHandle;
		std::basic_string<char16_t> _u16Path;
	};
}

#endif
