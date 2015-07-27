#pragma once
#ifndef DEBUGINFORENDER_H
#define DEBUGINFORENDER_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

#include "ToyGE\Kernel\Timer.h"

namespace ToyGE
{
	class RenderSharedEnviroment;
	class FontRenderer;

	class TOYGE_CORE_API DebugInfoRender : public RenderAction
	{
	public:
		DebugInfoRender();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<FontRenderer> _adapterInfoRenderer;
		Ptr<FontRenderer> _fpsRenderer;
		Ptr<FontRenderer> _dpRenderer;
		Ptr<FontRenderer> _cameraInfoRenderer;
	};
}

#endif 