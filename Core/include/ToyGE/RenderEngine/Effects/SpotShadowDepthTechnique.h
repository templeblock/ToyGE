#pragma once
#ifndef STANDARDSPOTSHADOWDEPTHTECHNIQUE_H
#define STANDARDSPOTSHADOWDEPTHTECHNIQUE_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	class SpotShadowDepthTechnique : public ShadowDepthTechnique
	{
	public:
		SpotShadowDepthTechnique();

		void RenderDepth(
			const Ptr<Texture> & shadowMap,
			const Ptr<LightComponent> & light,
			const Ptr<RenderSharedEnviroment> & sharedEnv) override;

		void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light) override;

		TextureType GetShadowMapTextureType() const override
		{
			return TEXTURE_2D;
		}

		int32_t GetShadowMapArraySize() const override
		{
			return 1;
		}

		//ShadowDepthTechniqueType Type() const
		//{
		//	return SHADOW_DEPTH_TECHNIQUE_STANTARD_SPOT;
		//}
	private:
		XMFLOAT4X4 _lightViewMat;
		XMFLOAT4X4 _lightProjMat;
		Ptr<RenderEffect> _fx;
	};
}

#endif