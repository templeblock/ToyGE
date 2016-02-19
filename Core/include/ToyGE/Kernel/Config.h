#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class TOYGE_CORE_API Config
	{
	public:
		String  windowTitle;
		int32_t windowWidth = 0;
		int32_t windowHeight = 0;
		int32_t windowX = 0;
		int32_t windowY = 0;

		int32_t adapterIndex = -1;
		String adapterSelectKey;
		bool bGraphicsEngineDebug = false;

		Config();

		static Ptr<Config> Load(const String & path);
	};
}

#endif