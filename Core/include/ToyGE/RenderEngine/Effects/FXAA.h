#pragma once
#ifndef FXAA_H
#define FXAA_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API FXAA : public RenderAction
	{
	public:
		FXAA();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _fx;
		float _fxaaConsoleRcpFrameOpt_N;
		float _fxaaQualitySubpix;
		float _fxaaQualityEdgeThreshold;
		float _fxaaQualityEdgeThresholdMin;
		float _fxaaConsoleEdgeSharpness;
		float _fxaaConsoleEdgeThreshold;
		float _fxaaConsoleEdgeThresholdMin;

		Ptr<Texture> SetupFXAAInTex(const Ptr<Texture> & inTex);
	};
}

#endif