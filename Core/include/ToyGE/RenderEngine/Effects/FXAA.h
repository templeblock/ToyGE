#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, FXAASetupPS, SHADER_PS, "FXAA", "FXAASetupPS", SM_4);
	DECLARE_SHADER(, FXAAPS, SHADER_PS, "FXAA", "FXAAPS", SM_4);

	class TOYGE_CORE_API FXAA : public RenderAction
	{
	public:
		FXAA();

		virtual void Render(const Ptr<RenderView> & view) override;

	private:
		float _fxaaConsoleRcpFrameOpt_N;
		float _fxaaQualitySubpix;
		float _fxaaQualityEdgeThreshold;
		float _fxaaQualityEdgeThresholdMin;
		float _fxaaConsoleEdgeSharpness;
		float _fxaaConsoleEdgeThreshold;
		float _fxaaConsoleEdgeThresholdMin;

		PooledTextureRef SetupFXAAInTex(const Ptr<Texture> & inTex);
	};
}

#endif