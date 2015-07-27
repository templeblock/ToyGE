#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class TOYGE_CORE_API Config
	{
	public:
		WString windowTitle;
		int32_t windowWidth;
		int32_t windowHeight;
		int32_t windowX;
		int32_t windowY;

		std::map<String, WString> resourceMap;

		Config();
		static bool Load(const WString & path, Config & outConfig);
	};
}

#endif