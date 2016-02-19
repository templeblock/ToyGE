#pragma once
#ifndef SHADOWTECHNIQUE_H
#define SHADOWTECHNIQUE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\RenderEngine\RenderView.h"

namespace ToyGE
{
	DECLARE_SHADER(, DepthOnlyVS, SHADER_VS, "DepthOnly", "DepthOnlyVS", SM_4);
	DECLARE_SHADER(, DepthOnlyPS, SHADER_PS, "DepthOnly", "DepthOnlyPS", SM_4);
	DECLARE_SHADER(, RenderRSMVS, SHADER_VS, "RenderRSM", "RenderRSMVS", SM_4);
	DECLARE_SHADER(, RenderRSMPS, SHADER_PS, "RenderRSM", "RenderRSMPS", SM_4);
	DECLARE_SHADER(, VSMConvertWithLinearizePS, SHADER_PS, "VSMConvert", "VSMConvertWithLinearizePS", SM_4);
	DECLARE_SHADER(, VSMConvertPS, SHADER_PS, "VSMConvert", "VSMConvertPS", SM_4);
	DECLARE_SHADER(, EVSM2ConvertWithLinearizePS, SHADER_PS, "EVSMConvert", "EVSM2ConvertWithLinearizePS", SM_4);
	DECLARE_SHADER(, EVSM2ConvertPS, SHADER_PS, "EVSMConvert", "EVSM2ConvertPS", SM_4);
	DECLARE_SHADER(, EVSM4ConvertWithLinearizePS, SHADER_PS, "EVSMConvert", "EVSM4ConvertWithLinearizePS", SM_4);
	DECLARE_SHADER(, EVSM4ConvertPS, SHADER_PS, "EVSMConvert", "EVSM4ConvertPS", SM_4);

	class LightComponent;
	class RenderView;
	class Camera;

	struct ReflectiveShadowMap
	{
		PooledTextureRef rsmRadiance;
		PooledTextureRef rsmDepth;
		PooledTextureRef rsmNormal;
	};

	class DepthDrawingPolicy : public DrawingPolicy
	{
	public:
		Ptr<RenderView> _view;
		Ptr<Material> _mat;

		virtual void BindView(const Ptr<class RenderView> & view) override;

		virtual void BindMaterial(const Ptr<class Material> & mat) override;

		virtual void Draw(const Ptr<class RenderComponent> & renderComponent) override;
	};

	class TOYGE_CORE_API ShadowTechnique : public StaticCastable
	{
	public:
		virtual ~ShadowTechnique() = default;

		void SetLight(const Ptr<LightComponent> & light)
		{
			_light = light;
		}
		Ptr<LightComponent> GetLight() const
		{
			return _light.lock();
		}

		CLASS_SET(ShadowMapSize, int2, _shadowMapSize);
		CLASS_GET(ShadowMapSize, int2, _shadowMapSize);

		CLASS_SET(RSMSize, int2, _rsmSize);
		CLASS_GET(RSMSize, int2, _rsmSize);

		virtual bool IsRelevantWithView() const
		{
			return false;
		}


		virtual void PrepareShadow(const Ptr<RenderView> & view);

		virtual void PrepareRSM(const Ptr<RenderView> & view);

		void ClearShadow()
		{
			_shadowMaps.clear();
		}

		void ClearRSM()
		{
			_rsms.rsmRadiance = nullptr;
			_rsms.rsmDepth = nullptr;
			_rsms.rsmNormal = nullptr;
		}

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view);

		const ReflectiveShadowMap & GetRSM() const
		{
			return _rsms;
		}

	protected:
		std::weak_ptr<LightComponent> _light;

		int2 _shadowMapSize = 512;
		int2 _rsmSize = 512;
		std::map<Ptr<RenderView>, PooledRenderResourceReference<Texture>> _shadowMaps;
		ReflectiveShadowMap _rsms;

		void RenderDepth(const Ptr<DepthStencilView> & dsv, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & viewPosTarget = nullptr);

		void RenderRSM(
			const Ptr<RenderView> & view, 
			const Ptr<RenderTargetView> & rsmRadiance,
			const Ptr<DepthStencilView> & rsmDepth,
			const Ptr<RenderTargetView> & rsmNormal);
	};

	template <int32_t numViews>
	struct ShadowView
	{
		XMFLOAT4X4 shadowWorldToViewMatrix[numViews];
		XMFLOAT4X4 shadowViewToClipMatrix[numViews];
		XMFLOAT4X4 shadowWorldToClipMatrix[numViews];
		float    shadowViewNear;
		float    shadowViewFar;
		float    shadowViewLength; // far - near
		float    _shadowpad_1;
		float2   shadowViewSize;
		float2   shadowInvViewSize;
	};

	class TOYGE_CORE_API SpotLightShadow : public ShadowTechnique
	{
	public:
		SpotLightShadow();

		virtual ~SpotLightShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

		CLASS_GET(ShadowView, ShadowView<1>, _spotShadowView);

	protected:
		Ptr<RenderView> _depthRenderView;
		ShadowView<1> _spotShadowView;
		Ptr<RenderBuffer> _spotShadowViewCB;
	};

	class TOYGE_CORE_API PointLightShadow : public ShadowTechnique
	{
	public:
		PointLightShadow();

		virtual ~PointLightShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

		CLASS_GET(ShadowView, ShadowView<6>, _pointShadowView);

	protected:
		Ptr<RenderView> _depthRenderView;
		ShadowView<6> _pointShadowView;
		Ptr<RenderBuffer> _pointShadowViewCB;
		bool _shadowMapBindAsCube = false;
	};


	namespace CascadedShadowMaxSplits
	{
		enum CacadedShadowMaxSplitsDef
		{
			VALUE = 4
		};
	}

	class TOYGE_CORE_API CascadedShadow : public ShadowTechnique
	{
	public:
		CascadedShadow();

		virtual ~CascadedShadow() = default;

		CLASS_SET(NumSplits, int32_t, _numSplits);
		CLASS_GET(NumSplits, int32_t, _numSplits);

		CLASS_SET(MaxDistance, float, _maxDistance);
		CLASS_GET(MaxDistance, float, _maxDistance);

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void PrepareRSM(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

		virtual bool IsRelevantWithView() const override
		{
			return true;
		}

		CLASS_GET(ShadowView, ShadowView<CascadedShadowMaxSplits::VALUE>, _cascadedShadowView);

		CLASS_GET(RSMView, ShadowView<1>, _cascadedRSMView);

	protected:
		ShadowView<CascadedShadowMaxSplits::VALUE> _cascadedShadowView;
		ShadowView<1> _cascadedRSMView;
		Ptr<RenderBuffer> _cascadedShadowViewCB;
		int32_t _numSplits = 3;
		std::vector<float> _splits;
		std::vector<float3> _splitsShadowViewMin;
		std::vector<float3> _splitsShadowViewMax;
		std::array<Ptr<RenderView>, CascadedShadowMaxSplits::VALUE> _depthRenderView;
		std::array<Ptr<RenderView>, 1> _rsmRenderView;
		float _maxDistance = 30.0f;
	};


	class TOYGE_CORE_API PCFShadow
	{
	public:
		virtual ~PCFShadow() = default;

		CLASS_SET(FilterSize, float2, _filterSize);
		CLASS_GET(FilterSize, float2, _filterSize);

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view);
	protected:
		float2 _filterSize = 2.0f;
	};
	
	class TOYGE_CORE_API SpotLightPCFShadow : public SpotLightShadow, public PCFShadow
	{
	public:
		virtual ~SpotLightPCFShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};

	class TOYGE_CORE_API PointLightPCFShadow : public PointLightShadow, public PCFShadow
	{
	public:
		virtual ~PointLightPCFShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};

	class TOYGE_CORE_API CascadedPCFShadow: public CascadedShadow, public PCFShadow
	{
	public:
		virtual ~CascadedPCFShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};


	class TOYGE_CORE_API VSMShadow
	{
	public:
		virtual ~VSMShadow() = default;

		CLASS_SET(FilterSize, float2, _filterSize);
		CLASS_GET(FilterSize, float2, _filterSize);

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view);
	protected:
		float2 _filterSize = 2.0f;
		int32_t _filterNumSamples = 7;
		float _vsmBias = 0.1f;
		float _vsmReduceBlending = 0.5f;
		RenderFormat _vsmFormat = RENDER_FORMAT_R32G32_FLOAT;

		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize);

		virtual void FilterVSM(const Ptr<Texture> & vsm, int32_t arrayIndex, float2 filterSize);
	};

	class TOYGE_CORE_API SpotLightVSMShadow : public SpotLightShadow, public virtual VSMShadow
	{
	public:
		virtual ~SpotLightVSMShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};

	class TOYGE_CORE_API PointLightVSMShadow : public PointLightShadow, public virtual VSMShadow
	{
	public:
		PointLightVSMShadow();

		virtual ~PointLightVSMShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};

	class TOYGE_CORE_API CascadedVSMShadow : public CascadedShadow, public virtual VSMShadow
	{
	public:
		virtual ~CascadedVSMShadow() = default;

		virtual void PrepareShadow(const Ptr<RenderView> & view) override;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;
	};


	class TOYGE_CORE_API EVSM2Shadow : public virtual VSMShadow
	{
	public:
		virtual ~EVSM2Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view);

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API SpotLightEVSM2Shadow : public SpotLightVSMShadow, public EVSM2Shadow
	{
	public:
		virtual ~SpotLightEVSM2Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API PointLightEVSM2Shadow : public PointLightVSMShadow, public EVSM2Shadow
	{
	public:
		virtual ~PointLightEVSM2Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API CascadedEVSM2Shadow : public CascadedVSMShadow, public EVSM2Shadow
	{
	public:
		virtual ~CascadedEVSM2Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};


	class TOYGE_CORE_API EVSM4Shadow : public virtual VSMShadow
	{
	public:
		virtual ~EVSM4Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view);

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API SpotLightEVSM4Shadow : public SpotLightVSMShadow, public EVSM4Shadow
	{
	public:
		SpotLightEVSM4Shadow()
		{
			_vsmFormat = RENDER_FORMAT_R32G32B32A32_FLOAT;
		}

		virtual ~SpotLightEVSM4Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API PointLightEVSM4Shadow : public PointLightVSMShadow, public EVSM4Shadow
	{
	public:
		PointLightEVSM4Shadow()
		{
			_vsmFormat = RENDER_FORMAT_R32G32B32A32_FLOAT;
		}

		virtual ~PointLightEVSM4Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};

	class TOYGE_CORE_API CascadedEVSM4Shadow : public CascadedVSMShadow, public EVSM4Shadow
	{
	public:
		CascadedEVSM4Shadow()
		{
			_vsmFormat = RENDER_FORMAT_R32G32B32A32_FLOAT;
		}

		virtual ~CascadedEVSM4Shadow() = default;

		virtual void BindMacros(const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<class Shader> & shader, const Ptr<RenderView> & view) override;

	protected:
		virtual void ConvertShadowMap(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target, bool bDoLinearize) override;
	};
}

#endif