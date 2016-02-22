#pragma once
#ifndef LIGHTCOMPONENT_H
#define LIGHTCOMPONENT_H

#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\Kernel\StaticCastable.h"

namespace ToyGE
{
	enum LightType : uint32_t
	{
		LIGHT_POINT			= 0UL,
		LIGHT_SPOT			= 1UL,
		LIGHT_DIRECTIONAL	= 2UL
	};

	namespace LightTypeNum
	{
		enum LightTypeNumDef
		{
			NUM = LIGHT_DIRECTIONAL + 1
		};
	}

	class RenderView;
	class Camera;
	class ShadowTechnique;

	class TOYGE_CORE_API LightComponent : public TransformComponent, public Cullable, public StaticCastable
	{
	public:
		LightComponent(LightType type);

		virtual ~LightComponent() = default;

		LightType Type() const
		{
			return _type;
		}

		CLASS_SET(Color, float3, _color);
		CLASS_GET(Color, float3, _color);

		void SetIntensity(float intensity);
		CLASS_GET(Intensity, float, _intensity);

		CLASS_SET(ShadowTechnique, Ptr<ShadowTechnique>, _shadowTechnique);
		CLASS_GET(ShadowTechnique, Ptr<ShadowTechnique>, _shadowTechnique);

		void SetCastShadow(bool bCastShadow)
		{
			_bCastShadow = bCastShadow;
		}
		bool IsCastShadow() const
		{
			return _bCastShadow;
		}

		/*void SetCastCaustics(bool bCastCaustics)
		{
			_bCastCaustics = bCastCaustics;
		}
		bool IsCastCaustics() const
		{
			return _bCastCaustics;
		}*/

		void SetCastLightVolume(bool bCastLightVolume)
		{
			_bCastLightVolume = bCastLightVolume;
		}
		bool IsCastLightVolume() const
		{
			return _bCastLightVolume;
		}

		void SetCastLPV(bool bCastLPV)
		{
			_bCastLPV = bCastLPV;
		}
		bool IsCastLPV() const
		{
			return _bCastLPV;
		}

		virtual float2 GetClipSpacePos(const Ptr<Camera> & camera) const;

		virtual void BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros);

		virtual void BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view);

	protected:
		LightType _type;

		float3 _color = 0.0f;
		float _intensity = 0.0f;
		bool _bCastShadow = false;
		Ptr<ShadowTechnique> _shadowTechnique;
		//bool _bCastCaustics = false;
		bool _bCastLightVolume = false;
		bool _bCastLPV = false;

		virtual void UpdateLightCuller();

		virtual void DoActive() override;

		virtual void OnTranformUpdated() override;
	};


	//PointLight
	class TOYGE_CORE_API PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent();

		float MaxDistance() const;

		virtual AABBox GetBoundsAABB() const override;

		Sphere GetBoundsSphere() const;

		virtual void BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view) override;
	};


	//SpotLight
	class TOYGE_CORE_API SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent();

		void SetDirection(const float3 & direction)
		{
			/*auto xmDir = XMLoadFloat3(&direction);
			xmDir = XMVector3Normalize(xmDir);
			XMStoreFloat3(&_direction, xmDir);*/
			_direction = direction;
		}

		const float3 & Direction() const
		{
			return _direction;
		}

		void SetDecreaseSpeed(float decreaseSpeed)
		{
			_decreaseSpeed = decreaseSpeed;
		}

		const float & DecreaseSpeed() const
		{
			return _decreaseSpeed;
		}

		float MaxAngle() const;

		float MaxDistance() const;

		AABBox GetBoundsAABB() const override;

		virtual void BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view) override;

	private:
		float3 _direction;
		float _decreaseSpeed;
	};


	//DirectionalLight
	class TOYGE_CORE_API DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent();

		void SetDirection(const float3 & direction)
		{
			/*auto xmDir = XMLoadFloat3(&direction);
			xmDir = XMVector3Normalize(xmDir);
			XMStoreFloat3(&_direction, xmDir);*/
			_direction = direction;
		}

		const float3 & Direction() const
		{
			return _direction;
		}

		void SetInfluenceAll(bool bInfluenceAll);

		bool IsInfluenceAll() const
		{
			return _bInfluenceAll;
		}

		void SetInfluenceArea(const AABBox & aabb)
		{
			_influenceAABB = aabb;
		}

		const AABBox & GetInfluenceAre() const
		{
			return _influenceAABB;
		}

		AABBox GetBoundsAABB() const override;

		CLASS_GET(Distance, float, _dist);
		CLASS_SET(Distance, float, _dist);

		float3 GetPosOffsetVec() const;

		virtual float2 GetClipSpacePos(const Ptr<Camera> & camera) const override;

		virtual void BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros) override;

		virtual void BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view) override;

	protected:
		void OnTranformUpdated() override;

	private:
		bool _bInfluenceAll;
		AABBox _influenceAABB;
		float3 _direction;
		float _dist;
	};
}

#endif