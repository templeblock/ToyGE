#pragma once
#ifndef FRAMEINFORENDERER_H
#define FRAMEINFORENDERER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class FontRenderer;

	class TOYGE_CORE_API FrameInfoRender
	{
	public:
		static Ptr<class Camera> traceCamera;

		static void Render(const Ptr<class RenderTargetView> & target);

	private:
	};
}

#endif 