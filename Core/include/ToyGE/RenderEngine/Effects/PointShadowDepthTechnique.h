#pragma once
#ifndef POINTSHADOWDEPTHTECHNIQUE_H
#define POINTSHADOWDEPTHTECHNIQUE_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	class PointShadowDepthTechnique : public ShadowDepthTechnique
	{
	public:
		PointShadowDepthTechnique();

		void RenderDepth(
			const Ptr<Texture> & shadowMap,
			const Ptr<LightComponent> & light,
			const Ptr<RenderSharedEnviroment> & sharedEnv,
			const std::array<Ptr<Texture>, 3> & rsm) override;

		void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light) override;

		TextureType GetShadowMapTextureType() const override
		{
			return TEXTURE_CUBE;
		}
		int32_t GetShadowMapArraySize() const override
		{
			return 1;
		}

		//ShadowDepthTechniqueType Type() const
		//{
		//	return SHADOW_DEPTH_TECHNIQUE_STANTARD_POINT;
		//}
	private:
		Ptr<RenderEffect> _fx;
		std::array<XMFLOAT4X4, 6> _pointLightViews;
		XMFLOAT4X4 _pointLightProj;
	};
}

#endif