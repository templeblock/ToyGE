#pragma once
#ifndef STRING_H
#define STRING_H

#include "ToyGE\Kernel\Endian.h"

namespace ToyGE
{
	template <typename T>
	static T FromBytes(const void * bytes)
	{
		T t;
		memcpy(&t, bytes, sizeof(T));
		return t;
	}

	template <typename T>
	static T FromBytes(const std::array<uint8_t, sizeof(T)> & bytes)
	{
		T t;
		memcpy(&t, &bytes[0], sizeof(T));
		return t;
	}


	enum class StringEncode
	{
		ANSI,
		UTF_8,
		UTF_16,
		UTF_32
	};

	template <StringEncode encode>
	class EncodeChar
	{
	};

	template <>
	class EncodeChar<StringEncode::ANSI>
	{
	public:
		using CharType = char;
	};

	template <>
	class EncodeChar<StringEncode::UTF_8>
	{
	public:
		using CharType = char;
	};

	template <>
	class EncodeChar<StringEncode::UTF_16>
	{
	public:
		using CharType = char16_t;
	};

	template <>
	class EncodeChar<StringEncode::UTF_32>
	{
	public:
		using CharType = char32_t;
	};


	template <StringEncode SrcEncode, StringEncode DstEncode, Endian endian>
	std::basic_string<typename EncodeChar<DstEncode>::CharType> StringConvert(const std::basic_string<typename EncodeChar<SrcEncode>::CharType> & srcStr)
	{
		return std::basic_string<typename EncodeChar<DstEncode>::CharType>();
	}

	template <StringEncode SrcEncode, StringEncode DstEncode>
	std::basic_string<typename EncodeChar<DstEncode>::CharType> StringConvert(const std::basic_string<typename EncodeChar<SrcEncode>::CharType> & srcStr)
	{
		return StringConvert<SrcEncode, DstEncode, EndianDef::DEF>(srcStr);
	}


	// ANSI to others
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::ANSI, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::ANSI, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char> & srcStr);

	// UTF8 to others
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_8, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_8, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char> & srcStr);

	// UTF16 to others
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_16, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char16_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_16, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char16_t> & srcStr);

	// UTF32 to others
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::ANSI, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::ANSI, Endian::BIG>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::UTF_8, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char> StringConvert<StringEncode::UTF_32, StringEncode::UTF_8, Endian::BIG>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_16, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char16_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_16, Endian::BIG>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_32, Endian::LITTLE>(const std::basic_string<char32_t> & srcStr);
	template <>
	TOYGE_CORE_API std::basic_string<char32_t> StringConvert<StringEncode::UTF_32, StringEncode::UTF_32, Endian::BIG>(const std::basic_string<char32_t> & srcStr);


	template <StringEncode encode>
	std::vector<std::basic_string<typename EncodeChar<encode>::CharType>> SplitString(const std::basic_string<typename EncodeChar<encode>::CharType> & str, const std::basic_string<typename EncodeChar<encode>::CharType> & split)
	{
		auto u32Str = StringConvert<encode, StringEncode::UTF_32>(str);
		auto u32Split = StringConvert<encode, StringEncode::UTF_32>(split);

		std::vector<std::basic_string<typename EncodeChar<encode>::CharType>> result;
		size_t offset = 0;
		size_t find = 0;
		while ((find = u32Str.find(u32Split, offset)) != std::string::npos)
		{
			result.push_back(StringConvert<StringEncode::UTF_32, encode>(u32Str.substr(offset, find - offset)));
			offset = find + split.size();
		}
		result.push_back(StringConvert<StringEncode::UTF_32, encode>(u32Str.substr(offset, find - offset)));
		return result;
	}


	TOYGE_CORE_API String FormatString(const char * format, va_list vl);
	TOYGE_CORE_API String FormatString(const char * format, ...);
}

#endif
