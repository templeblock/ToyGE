#pragma once
#ifndef SSAO_H
#define SSAO_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API SSAO : public RenderAction
	{
	public:
		SSAO();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _fx;

		void DownSampleNormalDepth(
			const Ptr<Texture> & inNormalTex,
			const Ptr<Texture> & inDepthTex,
			Ptr<Texture> & outNormalTex,
			Ptr<Texture> & outDepthTex);

		Ptr<Texture> RenderAOTex(
			const Ptr<Texture> & normalTex,
			const Ptr<Texture> & depthTex,
			const Ptr<Texture> & preMipAOTex,
			const Ptr<Texture> & preMipNormalTex,
			const Ptr<Texture> & preMipDepthTex,
			bool bUpSampling,
			bool bFullRes);

		//Ptr<Texture> BilateralUpSampling(const Ptr<Texture> & aoTex, const Ptr<Texture> & lowResDepthTex, const Ptr<Texture> & highResDepthTex);

		void CrossBilateralBlur(const Ptr<Texture> & aoTex, const Ptr<Texture> & depthTex);

		void CombineAO(const Ptr<Texture> & aoTex, const ResourceView & target);
	};
}

#endif