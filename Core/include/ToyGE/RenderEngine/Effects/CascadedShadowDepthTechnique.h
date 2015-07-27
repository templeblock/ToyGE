#pragma once
#ifndef CASCADEDSHADOWDEPTHTECHNIQUE_H
#define CASCADEDSHADOWDEPTHTECHNIQUE_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	/*
	PSSM
	*/
	class PSSMDepthTechnique : public ShadowDepthTechnique
	{
	public:
		PSSMDepthTechnique();

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
			return _numSplits;
		}

		bool IsRelevantWithCamera() const override
		{
			return true;
		}

		bool IsCSM() const override
		{
			return true;
		}

		//ShadowDepthTechniqueType Type() const
		//{
		//	return SHADOW_DEPTH_TECHNIQUE_PSSM;
		//}

		void SetNumSplits(int32_t numSplits);
		int32_t NumSplits() const
		{
			return _numSplits;
		}

		float SplitConfig(int32_t splitIndex) const
		{
			return _splitConfig[splitIndex];
		}
		const std::pair<float3, float3> & SplitMinMax(int32_t splitIndex) const
		{
			return _splitMinMax[splitIndex];
		}

	protected:
		virtual float2 GetNearFar(const Ptr<RenderSharedEnviroment> & sharedEnviroment) const;

	private:
		Ptr<RenderEffect> _fx;
		static int32_t _maxNumSplits;
		static int32_t _defaultNumSplits;
		int32_t _numSplits;
		std::vector<float> _splitConfig;
		std::vector<std::pair<float3, float3>> _splitMinMax;
		//std::vector<float> _lightMaxDist;
		XMFLOAT4X4 _lightViewMat;
		std::vector<XMFLOAT4X4> _lightCropMat;
	};


	/*
	SDSM
	*/
	class SDSMDepthTechnique : public PSSMDepthTechnique
	{
	public:
		SDSMDepthTechnique();

	protected:
		float2 GetNearFar(const Ptr<RenderSharedEnviroment> & sharedEnviroment) const override;

	private:
		Ptr<RenderEffect> _reduceMinMaxFX;
		int32_t _reduceBlockSize;

		float2 ReduceMinMax(const Ptr<Texture> & gbuffer) const;
	};
}

#endif