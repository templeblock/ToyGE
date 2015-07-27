#pragma once
#ifndef IOHELPER_H
#define IOHELPER_H

#include "ToyGE\Kernel\PreIncludes.h"

namespace ToyGE
{
	class TOYGE_CORE_API Reader
	{
	public:
		virtual ~Reader() = default;

		template <class T>
		T Read()
		{
			T ret;
			ReadBytes(&ret, sizeof(T));
			return ret;
		}

		void ReadBytes(void * dst, size_t numBytes)
		{
			DoReadBytes(dst, numBytes);
		}

		void ReadString(String & outStr)
		{
			char c;
			ReadBytes(&c, sizeof(char));
			while (c)
			{
				outStr.append(1, c);
				ReadBytes(&c, sizeof(char));
			}
		}

		void ReadString(WString & outStr)
		{
			wchar_t c;
			ReadBytes(&c, sizeof(wchar_t));
			while (c)
			{
				outStr.append(1, c);
				ReadBytes(&c, sizeof(wchar_t));
			}
		}

	protected:
		virtual void DoReadBytes(void * dst, size_t numBytes) = 0;
	};

	class TOYGE_CORE_API Writer
	{
	public:
		virtual ~Writer() = default;

		template <class T>
		void Write(const T & v)
		{
			WriteBytes(&v, sizeof(T));
		}

		void WriteBytes(const void * src, size_t numBytes)
		{
			DoWriteBytes(src, numBytes);
		}

		void WriteString(const String & str)
		{
			WriteBytes(str.c_str(), (str.size() + 1) * sizeof(char));
		}

		void WriteString(const WString & str)
		{
			WriteBytes(str.c_str(), (str.size() + 1) * sizeof(wchar_t));
		}

		void WriteStringNoTerminates(const String & str)
		{
			WriteBytes(str.c_str(), (str.size()) * sizeof(char));
		}

		void WriteStringNoTerminates(const WString & str)
		{
			WriteBytes(str.c_str(), (str.size()) * sizeof(wchar_t));
		}

	protected:
		virtual void DoWriteBytes(const void * src, size_t numBytes) = 0;
	};
}

#endif