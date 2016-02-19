#pragma once
#ifndef UTIL_H
#define UTIL_H

#include "ToyGE\Kernel\String.h"

namespace ToyGE
{
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

	TOYGE_CORE_API uint64_t Hash(const void * src, size_t size);
}

#endif