#pragma once
#ifndef DEFERREDSCENERENDERER_H
#define DEFERREDSCENERENDERER_H

#include "ToyGE\RenderEngine\SceneRenderer.h"

namespace ToyGE
{
	class TOYGE_CORE_API DeferredSceneRenderer : public SceneRenderer
	{
	public:
		DeferredSceneRenderer();

		virtual void Render(const Ptr<RenderView> & view) override;

	private:
		Ptr<RenderAction> _baseRender;
		Ptr<RenderAction> _ligtingRender;
		Ptr<RenderAction> _shadingRender;
		Ptr<RenderAction> _translucentRender;
	};
}

#endif