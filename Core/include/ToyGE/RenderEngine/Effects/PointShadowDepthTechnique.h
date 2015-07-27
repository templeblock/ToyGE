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
			const Ptr<RenderSharedEnviroment> & sharedEnv) override;

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
	};
}

#endif