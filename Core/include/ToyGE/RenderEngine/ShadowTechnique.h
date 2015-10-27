#pragma once
#ifndef SHADOWTECHNIQUE_H
#define SHADOWTECHNIQUE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Texture.h"

namespace ToyGE
{
	class RenderSharedEnviroment;
	class LightComponent;
	class Camera;
	class ShadowDepthTechnique;
	class ShadowRenderTechnique;
	class RenderEffect;
	class RasterizerState;

	class TOYGE_CORE_API ShadowTechnique
	{
	public:
		bool IsRelevantWithCamera() const;

		void PrepareShadow(const Ptr<LightComponent> & light, const Ptr<RenderSharedEnviroment> & sharedEnv);

		void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Camera> & camera);

		void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Camera> & camera);

		void SetShadowMapSize(const int2 & mapSize)
		{
			_shadowMapSize = mapSize;
		}

		const int2 & ShadowTexSize() const
		{
			return _shadowMapSize;
		}

		CLASS_SET(RSMSize, int2, _rsmSize);
		CLASS_GET(RSMSize, int2, _rsmSize);

		void SetDepthTechnique(const Ptr<ShadowDepthTechnique> & depthTech)
		{
			_depthTechnique = depthTech;
		}

		const Ptr<ShadowDepthTechnique> & DepthTechnique() const
		{
			return _depthTechnique;
		}

		void SetRenderTechnique(const Ptr<ShadowRenderTechnique> & renderTech)
		{
			_renderTechnique = renderTech;
		}

		const Ptr<ShadowRenderTechnique> & RenderTechnique() const
		{
			return _renderTechnique;
		}

		void SetRSM(const std::array<Ptr<Texture>, 3> & rsm)
		{
			_rsm = rsm;
		}

		const std::array<Ptr<Texture>, 3> & GetRSM() const
		{
			return _rsm;
		}

	private:
		Ptr<ShadowDepthTechnique> _depthTechnique;
		Ptr<ShadowRenderTechnique> _renderTechnique;
		int2 _shadowMapSize;
		Ptr<Texture> _shadowMap;
		std::map<Ptr<Camera>, Ptr<Texture>> _cameraRelevantShadowMap;
		Ptr<Texture> _rawShadowMap;
		std::map<Ptr<Camera>, Ptr<Texture>> _cameraRelevantRawShadowMap;
		int2 _rsmSize;
		std::array<Ptr<Texture>, 3> _rsm; //Depth, Normal, Flux
	};


	/*
	Depth Technique
	*/
	//enum ShadowDepthTechniqueType
	//{
	//	SHADOW_DEPTH_TECHNIQUE_STANTARD_POINT,
	//	SHADOW_DEPTH_TECHNIQUE_STANTARD_SPOT,
	//	SHADOW_DEPTH_TECHNIQUE_PSSM,
	//	SHADOW_DEPTH_TECHNIQUE_SDSM,
	//};

	class ShadowDepthTechnique
	{
	public:
		ShadowDepthTechnique();

		virtual ~ShadowDepthTechnique() = default;

		virtual void RenderDepth(
			const Ptr<Texture> & shadowMap,
			const Ptr<LightComponent> & light,
			const Ptr<RenderSharedEnviroment> & sharedEnv,
			const std::array<Ptr<Texture>, 3> & rsm) = 0;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light){};

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light){};

		virtual void BindRSMParams(const Ptr<RenderEffect> & fx) {};

		virtual TextureType GetShadowMapTextureType() const = 0;

		virtual int32_t GetShadowMapArraySize() const = 0;

		virtual bool IsRelevantWithCamera() const
		{
			return false;
		}

		virtual bool IsCSM() const
		{
			return false;
		}

		CLASS_SET(DepthBias, int32_t, _depthBias);
		CLASS_GET(DepthBias, int32_t, _depthBias);

		CLASS_SET(SlopedScaledDepthBias, float, _slopedScaledDepthBias);
		CLASS_GET(SlopedScaledDepthBias, float, _slopedScaledDepthBias);

	protected:
		int32_t _depthBias;
		float _slopedScaledDepthBias;
		Ptr<RasterizerState> _depthRenderRS;

		void UpdateDepthRasterizerState();
	};


	/*
	Render Technique
	*/
	//enum ShadowRenderTechniqueType
	//{
	//	SHADOW_RENDER_TECHNIQUE_VSM = 0,
	//	SHADOW_RENDER_TECHNIQUE_EVSM2 = 1,
	//	SHADOW_RENDER_TECHNIQUE_EVSM4 = 2,
	//	SHADOW_RENDER_TECHNIQUE_ESM = 3,
	//	SHADOW_RENDER_TECHNIQUE_SAVSM = 4,
	//	SHADOW_RENDER_TECHNIQUE_PCSS_SAVSM = 5
	//};

	class ShadowRenderTechnique : public std::enable_shared_from_this<ShadowRenderTechnique>
	{
	public:
		virtual ~ShadowRenderTechnique() = default;

		virtual Ptr<Texture> ProcessShadowTex(const Ptr<Texture> & shadowTex, const Ptr<LightComponent> & light) = 0;

		virtual void BindMacros(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap){};

		virtual void BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap);

		//virtual ShadowRenderTechniqueType Type() const = 0;

		template <class T>
		std::shared_ptr<T> As()
		{
			return std::static_pointer_cast<T>(shared_from_this());
		}

	};


	

	
}

#endif