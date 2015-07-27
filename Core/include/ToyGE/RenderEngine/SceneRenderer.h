#pragma once
#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class RenderView;
	class RenderAction;

	class TOYGE_CORE_API SceneRenderer
	{
	public:
		virtual ~SceneRenderer() = default;

		virtual void Render(const Ptr<RenderView> & view) = 0;

		CLASS_GET(BackgroundRender, Ptr<RenderAction>, _backgroundRender);
		CLASS_SET(BackgroundRender, Ptr<RenderAction>, _backgroundRender);

	protected:
		Ptr<RenderAction> _backgroundRender;
	};
}

#endif