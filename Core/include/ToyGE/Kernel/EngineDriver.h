#pragma once
#ifndef ENGINEDRIVER_H
#define ENGINEDRIVER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Kernel\Config.h"

namespace ToyGE
{
	class App;

	class TOYGE_CORE_API EngineDriver
	{
	public:
		static void StartUp(const Config & config, const Ptr<App> & app);

		static void Run();
	};
}

#endif