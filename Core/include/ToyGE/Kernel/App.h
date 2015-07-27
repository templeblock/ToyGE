#pragma once
#ifndef APP_H
#define APP_H

#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class TOYGE_CORE_API App
	{
	public:
		virtual ~App() = default;

		virtual void Startup(){}

		virtual void Update(float elapsedTime){}

		virtual void Destroy(){}
	};
}

#endif