#pragma once
#ifndef TWEAKBARRENDERER_H
#define TWEAKBARRENDERER_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API TweakBarRenderer : public RenderAction
	{
	public:
		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;
	};
}

#endif