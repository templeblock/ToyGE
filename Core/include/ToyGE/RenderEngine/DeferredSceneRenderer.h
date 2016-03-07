#pragma once
#ifndef DEFERREDSCENERENDERER_H
#define DEFERREDSCENERENDERER_H

#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"

namespace ToyGE
{
	DECLARE_SHADER(, DeferredRenderingBaseVS, SHADER_VS, "DeferredRenderingBase", "DeferredRenderingBaseVS", SM_4);
	DECLARE_SHADER(, DeferredRenderingBasePS, SHADER_PS, "DeferredRenderingBase", "DeferredRenderingBasePS", SM_4);
	DECLARE_SHADER(, DeferredRenderingLightingPS, SHADER_PS, "DeferredRenderingLightingPS", "DeferredRenderingLightingPS", SM_4);
	DECLARE_SHADER(, IBLPS, SHADER_PS, "DeferredRenderingLightingPS", "IBLPS", SM_4);
	DECLARE_SHADER(, DeferredRenderingShadingPS, SHADER_PS, "DeferredRenderingShadingPS", "DeferredRenderingShadingPS", SM_4);

	class TOYGE_CORE_API DeferredSceneRenderer : public SceneRenderer
	{
	public:
		virtual void Render(const Ptr<RenderView> & view) override;

	private:
		void InitBuffers(const Ptr<RenderView> & view);

		void RenderBase(const Ptr<RenderView> & view);

		void RenderLighting(const Ptr<RenderView> & view);

		void RenderShading(const Ptr<RenderView> & view);

	private:
		Ptr<class SkyBox> _skyBox;
		Ptr<class TranslucentRendering> _translucentRendering;
		Ptr<class EnvironmentReflectionRenderer> _envReflectionRenderer;
	};
}

#endif