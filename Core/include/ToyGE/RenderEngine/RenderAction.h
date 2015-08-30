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
		RenderAction()
			: _bEnable(true)
		{

		}

		virtual ~RenderAction() = default;

		virtual void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) = 0;

		CLASS_SET(Enable, bool, _bEnable);
		CLASS_GET(Enable, bool, _bEnable);

	protected:
		bool _bEnable;
	};


}

#endif