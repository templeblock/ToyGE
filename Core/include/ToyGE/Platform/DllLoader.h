#pragma once
#ifndef DLLLOADER_H
#define DLLLOADER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

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