#pragma once
#ifndef LIGHTCOMPONENT_H
#define LIGHTCOMPONENT_H

#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\SceneCuller.h"

namespace ToyGE
{
	enum LightType : uint32_t
	{
		LIGHT_POINT = 0UL,
		LIGHT_SPOT = 1UL,
		LIGHT_DIRECTIONAL = 2UL
	};

	namespace LightTypeNum
	{
		enum LightTypeNumDef
		{
			value = LIGHT_DIRECTIONAL + 1
		};
	}

	class Camera;
	class ShadowTechnique;
	class RenderEffect;

	class TOYGE_CORE_API LightComponent : public TransformComponent, public Cullable, public std::enable_shared_from_this<LightComponent>
	{
	public:
		LightComponent(LightType type);

		virtual ~LightComponent() = default;

		LightType Type() const
		{
			return _type;
		}

		void SetRadiance(const XMFLOAT3 & radiance);

		const XMFLOAT3 & Radiance() const
		{
			return _lightRadiance;
		}

		void SetCastShadow(bool bCastShadow)
		{
			_bCastShadow = bCastShadow;
		}

		bool IsCastShadow() const
		{
			return _bCastShadow;
		}

		void SetShadowTechnique(const Ptr<ShadowTechnique> & shadowTech)
		{
			_shadowTechnique = shadowTech;
		}

		const Ptr<ShadowTechnique> & GetShadowTechnique() const
		{
			return _shadowTechnique;
		}

		void SetCastCaustics(bool bCastCaustics)
		{
			_bCastCaustics = bCastCaustics;
		}

		bool IsCastCaustics() const
		{
			return _bCastCaustics;
		}

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

		virtual void BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera);

		virtual void BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera);

		virtual float2 GetLightClipPos(const Ptr<Camera> & camera) const;

	protected:
		bool _bCastShadow;
		Ptr<ShadowTechnique> _shadowTechnique;
		bool _bCastCaustics;
		bool _bCastLightVolume;
		bool _bCastLPV;

		void UpdateLightCuller();

		void DoActive() override;

		void OnTranformUpdated() override;

		virtual void OnRadianceChanged(const XMFLOAT3 & prevRadiance){};

	private:
		LightType _type;
		XMFLOAT3 _lightRadiance;
	};


	//PointLight
	class TOYGE_CORE_API PointLightComponent : public LightComponent
	{
	public:
		PointLightComponent();

		float MaxDistance() const;

		XNA::AxisAlignedBox GetBoundsAABB() const override;

		XNA::Sphere GetBoundsSphere() const;

		void BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		void BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		/*void StoreBoundsCache() override;

		bool Intersect(const XNA::AxisAlignedBox & aabb) const override;*/

	protected:
		void OnRadianceChanged(const XMFLOAT3 & prevRadiance) override;
	};


	//SpotLight
	class TOYGE_CORE_API SpotLightComponent : public LightComponent
	{
	public:
		SpotLightComponent();

		void SetDirection(const XMFLOAT3 & direction)
		{
			auto xmDir = XMLoadFloat3(&direction);
			xmDir = XMVector3Normalize(xmDir);
			XMStoreFloat3(&_direction, xmDir);
		}

		const XMFLOAT3 & Direction() const
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

		XNA::AxisAlignedBox GetBoundsAABB() const override;

		void BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		void BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		/*XNA::Frustum GetBoundsFrustum() const override;

		void StoreBoundsCache() override;

		bool Intersect(const XNA::AxisAlignedBox & aabb) const override;*/

	protected:
		void OnRadianceChanged(const XMFLOAT3 & prevRadiance) override;

	private:
		XMFLOAT3 _direction;
		float _decreaseSpeed;
	};

	//DirectionalLight
	class TOYGE_CORE_API DirectionalLightComponent : public LightComponent
	{
	public:
		DirectionalLightComponent();

		void SetDirection(const XMFLOAT3 & direction)
		{
			auto xmDir = XMLoadFloat3(&direction);
			xmDir = XMVector3Normalize(xmDir);
			XMStoreFloat3(&_direction, xmDir);
		}

		const XMFLOAT3 & Direction() const
		{
			return _direction;
		}

		void SetInfluenceAll(bool bInfluenceAll);

		bool IsInfluenceAll() const
		{
			return _bInfluenceAll;
		}

		void SetInfluenceArea(const XNA::AxisAlignedBox & aabb)
		{
			_influenceAABB = aabb;
		}

		const XNA::AxisAlignedBox & GetInfluenceAre() const
		{
			return _influenceAABB;
		}

		XNA::AxisAlignedBox GetBoundsAABB() const override;

		CLASS_GET(Distance, float, _dist);
		CLASS_SET(Distance, float, _dist);

		float3 GetPosOffsetVec() const;

		void BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		void BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera) override;

		/*void StoreBoundsCache() override;

		bool Intersect(const XNA::AxisAlignedBox & aabb) const override;*/

		float2 GetLightClipPos(const Ptr<Camera> & camera) const override;

	protected:
		void OnTranformUpdated() override;

	private:
		bool _bInfluenceAll;
		XNA::AxisAlignedBox _influenceAABB;
		XMFLOAT3 _direction;
		float _dist;
	};
}

#endif