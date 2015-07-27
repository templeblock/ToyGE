#include "ToyGE\Kernel\Util.h"
#include "boost\functional\hash.hpp"
#include <cstdio>

namespace ToyGE
{
#ifdef PLATFORM_WINDOWS
	void _ConvertStr_AToW_Windows(const String & src, WString & dst)
	{
		const int wStrLen = ::MultiByteToWideChar(CP_ACP, 0, src.c_str(), static_cast<int>(src.size()), nullptr, 0);
		std::vector<wchar_t> tmp(wStrLen);
		::MultiByteToWideChar(CP_ACP, 0, src.c_str(), static_cast<int>(src.size()), &tmp[0], wStrLen);

		dst.assign(tmp.begin(), tmp.end());
	}
#endif

	void ConvertStr_AToW(const String & src, WString & dst)
	{
#ifdef PLATFORM_WINDOWS
		_ConvertStr_AToW_Windows(src, dst);
#endif
	}

	void ConvertStr_WToA(const WString & src, String & dst)
	{
		dst.assign(src.begin(), src.end());
	}

	void IdenticalPath(WString & path)
	{
		for (auto itr = path.begin(); itr != path.end(); ++itr)
		{
			if (*itr == '/')
				*itr = '\\';
		}
	}

	void FormatString(String & outStr, const char * format, ...)
	{
		va_list vl;
		va_start(vl, format);
		FormatString(outStr, format, vl);
		va_end(vl);
	}

	void FormatString(String & outStr, const char * format, va_list vl)
	{
		std::stringstream ss;
		int cntC = _vscprintf(format, vl);
		if (cntC > 0)
		{
			auto buf = std::unique_ptr<char[]>(new char[cntC + 1]);
			buf[cntC] = 0;
			auto writeBytes = vsprintf_s(buf.get(), cntC + 1, format, vl);
			if (writeBytes == cntC)
			{
				ss << buf.get();
				outStr = ss.str();
			}
		}
	}

	static float GausscianFactor(float x)
	{
		return exp(x * x * -0.5f);
	}

	void GetGaussTable(int32_t radius, std::vector<float> & outTable)
	{
		float sum = 0.0f;
		//int blurSize = radius * 2 + 1;
		//int radius = tableSize / 2;
		outTable.clear();
		outTable.push_back(1.0f);
		for (int32_t i = 1; i <= radius; ++i)
		{
			float x = static_cast<float>(i) / static_cast<float>(radius) * 4.0f;
			float factor = GausscianFactor(x);
			sum += factor;
			outTable.push_back(factor);
		}
		sum = sum * 2.0f + 1.0f;

		for (int32_t i = 0; i <= radius; ++i)
		{
			outTable[i] /= sum;
		}
	}

	uint64_t Hash(const void * src, size_t size)
	{
		return boost::hash_range(static_cast<const uint8_t*>(src), static_cast<const uint8_t*>(src) + size);
	}

	uint64_t HashMacroDescs(const std::vector<MacroDesc> & macros)
	{
		uint64_t hashCode = 0;
		if (macros.size() > 0)
		{
			std::vector<MacroDesc> tmp = macros;
			std::sort(tmp.begin(), tmp.end(),
				[](MacroDesc & m0, MacroDesc & m1) -> bool
			{
				return m0.name < m1.name;
			});
			std::stringstream ss;
			for (auto & macro : tmp)
			{
				ss << macro.name << macro.value;
			}
			std::string strHash = ss.str();
			hashCode = boost::hash_range(strHash.begin(), strHash.end());
		}
		return hashCode;
	}
}