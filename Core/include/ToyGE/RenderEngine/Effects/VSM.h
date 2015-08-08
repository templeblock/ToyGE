#pragma once
#ifndef VSM_H
#define VSM_H

#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	/*
	VSM
	*/
	class ShadowRenderTechniqueVSM : public ShadowRenderTechnique
	{
	public:
		ShadowRenderTechniqueVSM();

		virtual ~ShadowRenderTechniqueVSM() = default;

		Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light) override;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//ShadowRenderTechniqueType Type() const override
		//{
		//	return SHADOW_RENDER_TECHNIQUE_VSM;
		//}

		CLASS_GET(Softness, float, _softness);
		CLASS_SET(Softness, float, _softness);

		CLASS_GET(Bias, float, _vsmBias);
		CLASS_SET(Bias, float, _vsmBias);

		CLASS_GET(ReduceBlending, float, _reduceBlending);
		CLASS_SET(ReduceBlending, float, _reduceBlending);

	protected:
		void ConvertShadowMap(const Ptr<Texture> & inTex, const Ptr<Texture> & outTex, const Ptr<RenderEffect> & fx, const String & techniqueName);

		Ptr<Texture> FilterShadowMap(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light);

	private:
		float _softness;
		float _vsmBias;
		float _reduceBlending;
	};


	/*
	EVSM2
	*/
	class ShadowRenderTechniqueEVSM2 : public ShadowRenderTechniqueVSM
	{
	public:
		virtual ~ShadowRenderTechniqueEVSM2() = default;

		Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light) override;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//ShadowRenderTechniqueType Type() const override
		//{
		//	return SHADOW_RENDER_TECHNIQUE_EVSM2;
		//}
	};


	/*
	EVSM4
	*/
	class ShadowRenderTechniqueEVSM4 : public ShadowRenderTechniqueEVSM2
	{
	public:
		Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light) override;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//ShadowRenderTechniqueType Type() const override
		//{
		//	return SHADOW_RENDER_TECHNIQUE_EVSM4;
		//}
	};


	/*
	SAVSM
	*/
	class ShadowRenderTechniqueSAVSM : public ShadowRenderTechnique
	{
	public:
		ShadowRenderTechniqueSAVSM();

		virtual ~ShadowRenderTechniqueSAVSM() = default;

		Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowMap, const Ptr<LightComponent> & light) override;

		//virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap) override;

		//ShadowRenderTechniqueType Type() const override
		//{
		//	return SHADOW_RENDER_TECHNIQUE_SAVSM;
		//}

		void SetFilterSize(float filterSize)
		{
			_filterSize = filterSize;
		}
		float FilterSize() const
		{
			return _filterSize;
		}

		Ptr<Texture> ShadowTex()
		{
			return _satTex;
		}

	protected:
		static float _defaultFilterSize;
		float _filterSize;
		Ptr<Texture> _satTex;
	};


	
}

#endif