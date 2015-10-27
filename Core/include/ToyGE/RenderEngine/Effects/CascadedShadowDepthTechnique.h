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
			const Ptr<RenderSharedEnviroment> & sharedEnv,
			const std::array<Ptr<Texture>, 3> & rsm) override;

		void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light) override;

		void BindRSMParams(const Ptr<RenderEffect> & fx) override;

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

		CLASS_SET(SplitMaxDist, float, _splitMaxDist);
		CLASS_GET(SplitMaxDist, float, _splitMaxDist);

	protected:
		virtual float2 GetNearFar(const Ptr<RenderSharedEnviroment> & sharedEnviroment) const;

	private:
		static int32_t _maxNumSplits;
		static int32_t _defaultNumSplits;

		Ptr<RenderEffect> _fx;
		int32_t _numSplits;
		float _splitMaxDist;
		std::vector<float> _splitConfig;
		std::vector<std::pair<float3, float3>> _splitMinMax;
		XMFLOAT4X4 _lightViewMat;
		std::vector<XMFLOAT4X4> _lightCropMat;
		XMFLOAT4X4 _rsmCropToWorldMat;
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