#pragma once
#ifndef RENDERACTION_H
#define RENDERACTION_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class RenderSharedEnviroment;
	class RenderEffect;
	class Texture;

	class TOYGE_CORE_API RenderAction
	{
	public:
		virtual ~RenderAction() = default;

		virtual void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) = 0;
	};


}

#endif