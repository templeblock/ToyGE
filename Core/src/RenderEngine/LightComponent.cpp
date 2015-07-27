#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Effects\PointShadowDepthTechnique.h"
#include "ToyGE\RenderEngine\Effects\SpotShadowDepthTechnique.h"
#include "ToyGE\RenderEngine\Effects\CascadedShadowDepthTechnique.h"
#include "ToyGE\RenderEngine\Effects\VSM.h"
#include "ToyGE\RenderEngine\RenderEffect.h"

namespace ToyGE
{
	LightComponent::LightComponent(LightType type)
		: _type(type),
		_bCastShadow(false),
		_bCastCaustics(false),
		_bCastLightVolume(false)
	{
	}

	void LightComponent::SetRadiance(const XMFLOAT3 & radiance)
	{
		if (_lightRadiance.x != radiance.x || _lightRadiance.y != radiance.y || _lightRadiance.z != radiance.z)
		{
			XMFLOAT3 prevRadiance = _lightRadiance;
			_lightRadiance = radiance;
			OnRadianceChanged(prevRadiance);
		}
	}

	void LightComponent::UpdateLightCuller()
	{
		if (IsActive())
			Global::GetRenderEngine()->GetSceneRenderLightsCuller()->UpdateElement(shared_from_this());
	}

	void LightComponent::BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		if (IsCastShadow() && !disableShadow)
		{
			effect->AddExtraMacro("LIGHT_SHADOW", "");
			GetShadowTechnique()->BindMacros(effect, shared_from_this(), camera);
		}
		else
		{
			effect->RemoveExtraMacro("LIGHT_SHADOW");
			effect->RemoveExtraMacro("SHADOW_TYPE");
		}
	}

	void LightComponent::BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		effect->VariableByName("lightRadiance")->AsScalar()->SetValue(&_lightRadiance);

		if (IsCastShadow() && !disableShadow)
			GetShadowTechnique()->BindParams(effect, shared_from_this(), camera);
	}

	void LightComponent::DoActive()
	{
		auto renderLightCuller = Global::GetRenderEngine()->GetSceneRenderLightsCuller();
		renderLightCuller->AddElement(shared_from_this());
	}

	void LightComponent::OnTranformUpdated()
	{
		UpdateLightCuller();
	}


	//PointLight
	PointLightComponent::PointLightComponent()
		: LightComponent(LIGHT_POINT)
	{
		_shadowTechnique = std::make_shared<ShadowTechnique>();
		_shadowTechnique->SetDepthTechnique(std::make_shared<PointShadowDepthTechnique>());
		_shadowTechnique->SetRenderTechnique(std::make_shared<ShadowRenderTechniqueEVSM4>());
		_shadowTechnique->SetShadowMapSize(256);
	}

	//static float _ComputePointLightMaxDistance(const XMFLOAT3 & radiance)
	//{
	//	float epison = 1e-5f;
	//	//auto & radiance = radiance;
	//	float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
	//	float maxDist = sqrtf(ilum / epison);
	//	return maxDist;
	//}

	float PointLightComponent::MaxDistance() const
	{
		float epison = 1e-3f;
		auto & radiance = Radiance();
		float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float maxDist = sqrtf(ilum / epison);

		return maxDist;
	}

	XNA::AxisAlignedBox PointLightComponent::GetBoundsAABB() const
	{
		XNA::AxisAlignedBox aabb;
		aabb.Center = GetPos();
		aabb.Extents.x = aabb.Extents.y = aabb.Extents.z = MaxDistance();
		return aabb;
	}

	XNA::Sphere PointLightComponent::GetBoundsSphere() const
	{
		XNA::Sphere sp;
		sp.Center = GetPos();
		sp.Radius = MaxDistance();
		return sp;
	}

	void PointLightComponent::BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindMacros(effect, disableShadow, camera);

		effect->AddExtraMacro("LIGHT_TYPE", "LIGHT_TYPE_POINT");
	}

	void PointLightComponent::BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindParams(effect, disableShadow, camera);

		effect->VariableByName("lightPos")->AsScalar()->SetValue(&_pos);
	}

	void PointLightComponent::OnRadianceChanged(const XMFLOAT3 & prevRadiance)
	{
		UpdateLightCuller();
	}


	//SpotLight
	SpotLightComponent::SpotLightComponent()
		: LightComponent(LIGHT_SPOT)
	{
		_shadowTechnique = std::make_shared<ShadowTechnique>();
		_shadowTechnique->SetDepthTechnique(std::make_shared<SpotShadowDepthTechnique>());
		_shadowTechnique->SetRenderTechnique(std::make_shared<ShadowRenderTechniqueEVSM4>());
		_shadowTechnique->SetShadowMapSize(256);
	}

	float SpotLightComponent::MaxAngle() const
	{
		auto & radiance = Radiance();
		float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float epison = 0.01f;
		return acosf(powf(epison / ilum, 1.0f / DecreaseSpeed()));
	}

	float SpotLightComponent::MaxDistance() const
	{
		float epison = 0.01f;
		auto & radiance = Radiance();
		float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float maxDist = sqrtf(ilum / epison);

		return maxDist;
	}

	XNA::AxisAlignedBox SpotLightComponent::GetBoundsAABB() const
	{
		XNA::AxisAlignedBox aabb;
		float3 pos = *(reinterpret_cast<const float3*>(&GetPos()));
		float3 dir = *(reinterpret_cast<const float3*>(&Direction()));
		float3 u = abs(dir.y) > 0.99f ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);
		float3 r = normalize(cross(u, dir));
		u = cross(dir, r);
		float maxDist = MaxDistance();
		float t = tan(MaxAngle()) * maxDist;
		float3 v0 = pos;
		float3 v1 = pos + dir * maxDist;
		float3 v2 = v1 + u * t;
		float3 v3 = v1 - u * t;
		float3 v4 = v1 + r * t;
		float3 v5 = v1 - r * t;
		float3 vMin = vecMin({ v0, v1, v2, v3, v4, v5 });
		float3 vMax = vecMax({ v0, v1, v2, v3, v4, v5 });
		Math::MinMaxToAxisAlignedBox(vMin, vMax, aabb);

		return aabb;
	}

	void SpotLightComponent::BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindMacros(effect, disableShadow, camera);

		effect->AddExtraMacro("LIGHT_TYPE", "LIGHT_TYPE_SPOT");
	}

	void SpotLightComponent::BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindParams(effect, disableShadow, camera);

		effect->VariableByName("lightPos")->AsScalar()->SetValue(&_pos);
		effect->VariableByName("lightDir")->AsScalar()->SetValue(&_direction);
	}

	void SpotLightComponent::OnRadianceChanged(const XMFLOAT3 & prevRadiance)
	{
		UpdateLightCuller();
	}


	//DirectionalLight
	DirectionalLightComponent::DirectionalLightComponent()
		: LightComponent(LIGHT_DIRECTIONAL),
		_bInfluenceAll(true),
		_dist(5000.0f)
	{
		memset(&_influenceAABB, 0, sizeof(_influenceAABB));

		_shadowTechnique = std::make_shared<ShadowTechnique>();
		_shadowTechnique->SetDepthTechnique(std::make_shared<SDSMDepthTechnique>());
		auto evsm = std::make_shared<ShadowRenderTechniqueEVSM2>();
		evsm->SetSoftness(0.08f);
		_shadowTechnique->SetRenderTechnique(evsm);
		_shadowTechnique->SetShadowMapSize(1024);
	}

	void DirectionalLightComponent::SetInfluenceAll(bool bInfluenceAll)
	{
		if (_bInfluenceAll != bInfluenceAll && IsActive())
			Global::GetRenderEngine()->GetSceneRenderLightsCuller()->UpdateElement(shared_from_this());

		_bInfluenceAll = bInfluenceAll;
		if (_bInfluenceAll)
			memset(&_influenceAABB, 0, sizeof(_influenceAABB));
	}

	XNA::AxisAlignedBox DirectionalLightComponent::GetBoundsAABB() const
	{
		if (_bInfluenceAll)
			return XNA::AxisAlignedBox{ { 0.0f, 0.0f, 0.0f }, { FLT_MAX * 0.5f, FLT_MAX * 0.5f, FLT_MAX * 0.5f } };
		else
			return _influenceAABB;
	}

	void DirectionalLightComponent::BindMacros(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindMacros(effect, disableShadow, camera);

		effect->AddExtraMacro("LIGHT_TYPE", "LIGHT_TYPE_DIRECTIONAL");
	}

	void DirectionalLightComponent::BindParams(const Ptr<RenderEffect> & effect, bool disableShadow, const Ptr<Camera> & camera)
	{
		LightComponent::BindParams(effect, disableShadow, camera);

		effect->VariableByName("lightDir")->AsScalar()->SetValue(&_direction);
	}

	void DirectionalLightComponent::OnTranformUpdated()
	{
		if (_bInfluenceAll)
			return;
		else
			LightComponent::OnTranformUpdated();
	}

	float3 DirectionalLightComponent::GetPosOffsetVec() const
	{
		float3 dir = *reinterpret_cast<const float3*>(&_direction);
		return dir * -_dist;
	}
}