#pragma once
#ifndef DLLLOADER_H
#define DLLLOADER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class DllObj;

	class TOYGE_CORE_API DllLoader
	{
	public:
		virtual ~DllLoader() = default;

		virtual Ptr<DllObj> LoadDll(const String & name) = 0;
	};
}

#endif