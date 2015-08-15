#include "ToyGE\RenderEngine\DeferredSceneRenderer.h"
#include "ToyGE\RenderEngine\Effects\DeferredRendering.h"
#include "ToyGE\RenderEngine\Effects\TranslucencyRendering.h"
#include "ToyGE\RenderEngine\RenderView.h"

namespace ToyGE
{
	DeferredSceneRenderer::DeferredSceneRenderer()
	{
		_baseRender = std::make_shared<DeferredRenderingBase>();
		_ligtingRender = std::make_shared<DeferredRenderingLighting>();
		_shadingRender = std::make_shared<DeferredRenderingShading>();
		_translucentRender = std::make_shared<TranslucencyRendering>();
	}

	void DeferredSceneRenderer::RenderBase(const Ptr<RenderView> & view)
	{
		_baseRender->Render(view->GetRenderSharedEnviroment());
	}

	void DeferredSceneRenderer::RenderShading(const Ptr<RenderView> & view)
	{
		_ligtingRender->Render(view->GetRenderSharedEnviroment());
		_shadingRender->Render(view->GetRenderSharedEnviroment());
		if (_backgroundRender)
			_backgroundRender->Render(view->GetRenderSharedEnviroment());
		_translucentRender->Render(view->GetRenderSharedEnviroment());
	}
}