#pragma once
#ifndef RNEDERCONFIG_H
#define RNEDERCONFIG_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class TOYGE_CORE_API RenderConfig
	{
	public:
		std::map<String, String> configMap;
	};
}

#endif