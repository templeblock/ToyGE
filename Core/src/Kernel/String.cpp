#include "ToyGE\Kernel\String.h"

namespace ToyGE
{
	// ANSI to others
	template <>
	std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return std::basic_string<char16_t>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return std::basic_string<char16_t>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return std::basic_string<char32_t>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return std::basic_string<char32_t>(srcStr.begin(), srcStr.end());
	}

	// UTF8 to others
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8_utf16<uint16_t, 0x10ffff, std::little_endian>, uint16_t> cvt;
		auto cvtResult = cvt.from_bytes(srcStr);
		return std::basic_string<char16_t>(cvtResult.begin(), cvtResult.end());
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> cvt;
		auto cvtResult = cvt.from_bytes(srcStr);
		return std::basic_string<char16_t>(cvtResult.begin(), cvtResult.end());
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8<uint32_t, 0x10ffff, std::little_endian>, uint32_t> cvt;
		auto cvtResult = cvt.from_bytes(srcStr);
		return std::basic_string<char32_t>(cvtResult.begin(), cvtResult.end());
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> cvt;
		auto cvtResult = cvt.from_bytes(srcStr);
		return std::basic_string<char32_t>(cvtResult.begin(), cvtResult.end());
	}

	// UTF16 to others
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr)
	{
		return std::basic_string<char>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char16_t> & srcStr)
	{
		return std::basic_string<char>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8_utf16<uint16_t, 0x10ffff, std::little_endian>, uint16_t> cvt;
		return cvt.to_bytes(std::basic_string<uint16_t>(srcStr.begin(), srcStr.end()));
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char16_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t> cvt;
		return cvt.to_bytes(std::basic_string<uint16_t>(srcStr.begin(), srcStr.end()));
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char16_t> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf16<uint32_t, 0x10ffff, std::little_endian>, uint32_t> cvt;
		auto cvtResult = cvt.from_bytes((char*)srcStr.c_str());
		return std::basic_string<char32_t>(cvtResult.begin(), cvtResult.end());
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char16_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> cvt;
		auto cvtResult = cvt.from_bytes((char*)srcStr.c_str());
		return std::basic_string<char32_t>(cvtResult.begin(), cvtResult.end());
	}

	// UTF32 to others
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr)
	{
		return std::basic_string<char>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char32_t> & srcStr)
	{
		return std::basic_string<char>(srcStr.begin(), srcStr.end());
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8<uint32_t, 0x10ffff, std::little_endian>, uint32_t> cvt;
		return cvt.to_bytes(std::basic_string<uint32_t>(srcStr.begin(), srcStr.end()));
	}
	template <>
	std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char32_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> cvt;
		return cvt.to_bytes(std::basic_string<uint32_t>(srcStr.begin(), srcStr.end()));
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf16<uint32_t, 0x10ffff, std::little_endian>, uint32_t> cvt;
		auto cvtResult = cvt.to_bytes(std::basic_string<uint32_t>(srcStr.begin(), srcStr.end()));
		std::basic_string<char16_t> result(cvtResult.size() >> 1, 0);
		for (size_t i = 0; i < cvtResult.size(); i += 2)
			result[i >> 1] = FromBytes<char16_t>(&cvtResult[i]);
		return result;
	}
	template <>
	std::basic_string<char16_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char32_t> & srcStr)
	{
		static std::wstring_convert<std::codecvt_utf16<uint32_t>, uint32_t> cvt;
		auto cvtResult = cvt.to_bytes(std::basic_string<uint32_t>(srcStr.begin(), srcStr.end()));
		std::basic_string<char16_t> result(cvtResult.size() >> 1, 0);
		for (size_t i = 0; i < cvtResult.size(); i += 2)
			result[i >> 1] = FromBytes<char16_t>(&cvtResult[i]);
		return result;
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr)
	{
		return srcStr;
	}
	template <>
	std::basic_string<char32_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char32_t> & srcStr)
	{
		return srcStr;
	}

	String FormatString(const char * format, va_list vl)
	{
		auto cnt = vsnprintf(nullptr, 0, format, vl);
		if (cnt > 0)
		{
			std::vector<char> buf(cnt + 1);
			auto writeBytes = vsnprintf(&buf[0], cnt + 1, format, vl);
			if (writeBytes == cnt)
				return String(&buf[0]);
		}

		return String();
	}

	String FormatString(const char * format, ...)
	{
		va_list vl;
		va_start(vl, format);
		auto str = FormatString(format, vl);
		va_end(vl);
		return str;
	}
}