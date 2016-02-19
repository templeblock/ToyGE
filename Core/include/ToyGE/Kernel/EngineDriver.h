#pragma once
#ifndef ENGINEDRIVER_H
#define ENGINEDRIVER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class App;

	class TOYGE_CORE_API EngineDriver
	{
	public:
		static void Init(const Ptr<App> & app);

		static void Run();
	};
}

#endif