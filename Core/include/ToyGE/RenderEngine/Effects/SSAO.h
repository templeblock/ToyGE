#pragma once
#ifndef SSAO_H
#define SSAO_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	DECLARE_SHADER(, DownSampleNormalDepthPS, SHADER_PS, "SSAO", "DownSampleNormalDepthPS", SM_4);
	DECLARE_SHADER(, RenderAOPS, SHADER_PS, "SSAO", "RenderAOPS", SM_4);
	DECLARE_SHADER(, CrossBilateralBlurXPS, SHADER_PS, "SSAO", "CrossBilateralBlurXPS", SM_4);
	DECLARE_SHADER(, CrossBilateralBlurYPS, SHADER_PS, "SSAO", "CrossBilateralBlurYPS", SM_4);
	DECLARE_SHADER(, CombineAOPS, SHADER_PS, "SSAO", "CombineAOPS", SM_4);

	class TOYGE_CORE_API SSAO : public RenderAction
	{
	public:
		SSAO();

		void Render(const Ptr<RenderView> & view) override;

		CLASS_SET(AORadius, float, _aoRadius)
		CLASS_GET(AORadius, float, _aoRadius)

		CLASS_SET(AOPower, float, _aoPower)
		CLASS_GET(AOPower, float, _aoPower)

		CLASS_SET(AOIntensity, float, _aoIntensity)
		CLASS_GET(AOIntensity, float, _aoIntensity)

	private:
		float _aoRadius;
		float _aoPower;
		float _aoIntensity;

		void DownSampleNormalDepth(
			const Ptr<RenderView> & view,
			const Ptr<Texture> & inNormalTex,
			const Ptr<Texture> & inDepthTex,
			PooledTextureRef & outNormalTex,
			PooledTextureRef & outDepthTex);

		PooledTextureRef RenderAOTex(
			float aoRadius,
			const Ptr<RenderView> & view,
			const Ptr<Texture> & normalTex,
			const Ptr<Texture> & depthTex,
			const Ptr<Texture> & preMipAOTex,
			const Ptr<Texture> & preMipNormalTex,
			const Ptr<Texture> & preMipDepthTex,
			bool bUpSampling,
			bool bFullRes);

		void CrossBilateralBlur(const Ptr<RenderView> & view, const Ptr<Texture> & aoTex, const Ptr<Texture> & depthTex);

		void CombineAO(const Ptr<Texture> & aoTex, const Ptr<Texture> & target);
	};
}

#endif