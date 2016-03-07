#pragma once
#ifndef TRANSLUCENTRENDERING_H
#define TRANSLUCENTRENDERING_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, ForwardTranslucentRenderingVS, SHADER_VS, "ForwardTranslucentRendering", "ForwardTranslucentRenderingVS", SM_4);
	DECLARE_SHADER(, ForwardTranslucentRenderingPS, SHADER_PS, "ForwardTranslucentRendering", "ForwardTranslucentRenderingPS", SM_4);
	DECLARE_SHADER(, ForwardTranslucentLightingPS, SHADER_PS, "ForwardTranslucentRendering", "ForwardTranslucentLightingPS", SM_4);
	DECLARE_SHADER(, ForwardTranslucentAmbientPS, SHADER_PS, "ForwardTranslucentRendering", "ForwardTranslucentAmbientPS", SM_4);
	DECLARE_SHADER(, ForwardTranslucentRenderingVelocityPS, SHADER_PS, "ForwardTranslucentRendering", "ForwardTranslucentRenderingVelocityPS", SM_4);
	DECLARE_SHADER(, OITRenderingVS, SHADER_VS, "OITRendering", "OITRenderingVS", SM_4);
	DECLARE_SHADER(, OITLightingPS, SHADER_PS, "OITRendering", "OITLightingPS", SM_4);
	DECLARE_SHADER(, OITAmbientPS, SHADER_PS, "OITRendering", "OITAmbientPS", SM_4);
	DECLARE_SHADER(, OITRenderingPS, SHADER_PS, "OITRendering", "OITRenderingPS", SM_4);
	DECLARE_SHADER(, OITRenderingMergeCS, SHADER_CS, "OITRenderingMergeCS", "OITRenderingMergeCS", SM_5);
	/*DECLARE_SHADER(, CausticsVS, SHADER_VS, "Caustics", "CausticsVS", SM_4);
	DECLARE_SHADER(, CausticsGS, SHADER_GS, "Caustics", "CausticsGS", SM_4);
	DECLARE_SHADER(, CausticsPS, SHADER_PS, "Caustics", "CausticsPS", SM_4);*/

	class RenderBuffer;
	class Texture;

	class TOYGE_CORE_API TranslucentRendering
	{
	public:
		bool bOIT = true;

		virtual ~TranslucentRendering() = default;

		virtual void Render(
			const Ptr<class RenderView> & view,
			const Ptr<class RenderTargetView> & target);

	protected:
		

		void ForwardRender(
			const Ptr<class RenderView> & view,
			const Ptr<class RenderTargetView> & target);

		void OITRender(
			const Ptr<class RenderView> & view,
			const Ptr<class RenderTargetView> & target);
	};
}

#endif
