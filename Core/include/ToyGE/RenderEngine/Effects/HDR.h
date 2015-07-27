#pragma once
#ifndef HDR_H
#define HDR_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API HDR : public RenderAction
	{
	public:
		HDR();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

		CLASS_GET(BrightPassThreshold, float, _brightPassThreshold);
		CLASS_SET(BrightPassThreshold, float, _brightPassThreshold);

		CLASS_GET(BrightPassScaleParam, float, _brightPassScaleParam);
		CLASS_SET(BrightPassScaleParam, float, _brightPassScaleParam);

	private:
		Ptr<RenderEffect> _hdrFX;
		Ptr<Texture> _prevAvgAdaptedIlumTex;
		Ptr<Texture> _avgAdaptedIlumTex;
		float _brightPassThreshold;
		float _brightPassScaleParam;

		Ptr<Texture> SceneDownSample(const Ptr<Texture> & scene);

		void CalcIlluminace(const Ptr<Texture> & downSampleTex);

		Ptr<Texture> BrightPass(const Ptr<Texture> & downSampleTex);

		Ptr<Texture> BloomDownSample(const Ptr<Texture> & inTex);

		void BloomUpSample(const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex);

		void Combine(
			const Ptr<Texture> & scene,
			const Ptr<Texture> & blurTex,
			const Ptr<Texture> & streakTex,
			const ResourceView & target);
	};
}

#endif