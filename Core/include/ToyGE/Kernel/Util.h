#pragma once
#ifndef UTIL_H
#define UTIL_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	void TOYGE_CORE_API ConvertStr_AToW(const String & src, WString & dst);

	void TOYGE_CORE_API ConvertStr_WToA(const WString & src, String & dst);

	template <class T>
	inline std::shared_ptr<T>
		MakeComShared(T * pCom)
	{
		return pCom ? std::shared_ptr<T>(pCom, std::mem_fn(&T::Release)) : std::shared_ptr<T>();
	}

	inline std::shared_ptr<uint8_t> MakeBufferedDataShared(size_t bufferSize)
	{
		return
			bufferSize > 0 ?
			std::shared_ptr<uint8_t>(new uint8_t[bufferSize], [](uint8_t * pData){ delete[] pData; })
			: std::shared_ptr<uint8_t>();
	}

	inline std::unique_ptr<uint8_t[]> MakeBufferedData(size_t bufferSize)
	{
		return
			bufferSize > 0 ?
			std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize])
			: std::unique_ptr<uint8_t[]>();
	}
	
	template <char c0, char c1, char c2, char c3>
	class MakeFourCC
	{
	public:
		enum { value = (((uint32_t)(uint8_t)(c0)) | ((uint32_t)(uint8_t)(c1) << 8) | ((uint32_t)(uint8_t)(c2) << 16) | ((uint32_t)(uint8_t)(c3) << 24)) };
	};

	void TOYGE_CORE_API IdenticalPath(WString & path);

	void TOYGE_CORE_API FormatString(String & outStr, const char * format, ...);
	void TOYGE_CORE_API FormatString(String & outStr, const char * format, va_list vl);

	void TOYGE_CORE_API GetGaussTable(int32_t radius, std::vector<float> & outTable);

	uint64_t TOYGE_CORE_API Hash(const void * src, size_t size);

	uint64_t TOYGE_CORE_API HashMacroDescs(const std::vector<MacroDesc> & macros);

}

#endif