#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	LightComponent::LightComponent(LightType type)
		: _type(type)
	{
	}

	void LightComponent::SetIntensity(float intensity)
	{
		_intensity = intensity;
		UpdateLightCuller();
	}

	void LightComponent::UpdateLightCuller()
	{
		if (IsActive())
			Global::GetRenderEngine()->GetSceneRenderLightsCuller()->UpdateElement(shared_from_this()->Cast<LightComponent>());
	}

	float2 LightComponent::GetClipSpacePos(const Ptr<Camera> & camera) const
	{
		auto posW = this->GetPos();

		/*auto posWXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>( &posW));
		auto viewXM = XMLoadFloat4x4(&camera->GetViewMatrix());
		auto projXM = XMLoadFloat4x4(&camera->GetProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
		auto posHXM = XMVector3TransformCoord(posWXM, viewProjXM);*/
		auto posH = transform_coord(posW, camera->GetViewProjMatrix());
		/*XMFLOAT4 posH;
		XMStoreFloat4(&posH, posHXM);*/
		/*posH.x() /= posH.w;
		posH.y() /= posH.w;*/

		return float2(posH.x(), posH.y());
	}

	void LightComponent::BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		if (IsCastShadow() && enableShadow && _shadowTechnique)
		{
			_shadowTechnique->BindMacros(view, outMacros);
		}
	}

	void LightComponent::BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view)
	{
		shader->SetScalar("lightRadiance", _color * _intensity);

		if (IsCastShadow() && enableShadow && _shadowTechnique)
		{
			_shadowTechnique->BindShaderParams(shader, view);
		}
	}

	void LightComponent::DoActive()
	{
		auto renderLightCuller = Global::GetRenderEngine()->GetSceneRenderLightsCuller();
		renderLightCuller->AddElement(shared_from_this()->Cast<LightComponent>());
	}

	void LightComponent::OnTranformUpdated()
	{
		UpdateLightCuller();
	}

	


	//PointLight
	PointLightComponent::PointLightComponent()
		: LightComponent(LIGHT_POINT)
	{
		_shadowTechnique = std::make_shared<PointLightPCFShadow>();
		_shadowTechnique->SetShadowMapSize(128);
	}

	float PointLightComponent::MaxDistance() const
	{
		float epison = 1e-3f;
		//auto & radiance = Radiance();
		//float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float maxDist = sqrtf(_intensity / epison);

		return maxDist;
	}

	AABBox PointLightComponent::GetBoundsAABB() const
	{
		/*XNA::AxisAlignedBox aabb;
		aabb.Center = GetPos();
		aabb.Extents.x = aabb.Extents.y = aabb.Extents.z = MaxDistance();*/
		AABBox aabb;
		aabb.min = GetPos() - MaxDistance();
		aabb.max = GetPos() + MaxDistance();
		return aabb;
	}

	Sphere PointLightComponent::GetBoundsSphere() const
	{
		Sphere sp(GetPos(), MaxDistance());

		/*sp.Center = GetPos();
		sp.Radius = MaxDistance();*/
		return sp;
	}

	void PointLightComponent::BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		LightComponent::BindMacros(enableShadow, view, outMacros);

		outMacros["LIGHT_TYPE"] = "LIGHT_TYPE_POINT";
	}

	void PointLightComponent::BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view)
	{
		LightComponent::BindShaderParams(shader, enableShadow, view);

		shader->SetScalar("lightPos", _pos);
	}


	//SpotLight
	SpotLightComponent::SpotLightComponent()
		: LightComponent(LIGHT_SPOT)
	{
		_shadowTechnique = std::make_shared<SpotLightPCFShadow>();
		_shadowTechnique->SetShadowMapSize(512);
	}

	float SpotLightComponent::MaxAngle() const
	{
		//auto & radiance = Radiance();
		//float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float epison = 0.01f;
		return acosf(powf(epison / _intensity, 1.0f / DecreaseSpeed()));
	}

	float SpotLightComponent::MaxDistance() const
	{
		float epison = 0.01f;
		//auto & radiance = Radiance();
		//float ilum = 0.21f * radiance.x + 0.72f * radiance.y + 0.07f * radiance.z;
		float maxDist = sqrtf(_intensity / epison);

		return maxDist;
	}

	AABBox SpotLightComponent::GetBoundsAABB() const
	{
		float3 pos = *(reinterpret_cast<const float3*>(&GetPos()));
		float3 dir = *(reinterpret_cast<const float3*>(&Direction()));
		float3 u = abs(dir.y()) > 0.99f ? float3(1.0f, 0.0f, 0.0f) : float3(0.0f, 1.0f, 0.0f);
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
		float3 vMin = min_vec({ v0, v1, v2, v3, v4, v5 });
		float3 vMax = max_vec({ v0, v1, v2, v3, v4, v5 });
		AABBox aabb(vMin, vMax);
		//Math::MinMaxToAxisAlignedBox(vMin, vMax, aabb);

		return aabb;
	}

	void SpotLightComponent::BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		LightComponent::BindMacros(enableShadow, view, outMacros);

		outMacros["LIGHT_TYPE"] = "LIGHT_TYPE_SPOT";
	}

	void SpotLightComponent::BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view)
	{
		LightComponent::BindShaderParams(shader, enableShadow, view);

		shader->SetScalar("lightPos", _pos);
		shader->SetScalar("lightDir", _direction);
		shader->SetScalar("spotLightDecreaseSpeed", _decreaseSpeed);
	}


	//DirectionalLight
	DirectionalLightComponent::DirectionalLightComponent()
		: LightComponent(LIGHT_DIRECTIONAL),
		_bInfluenceAll(true),
		_dist(5000.0f)
	{
		memset(&_influenceAABB, 0, sizeof(_influenceAABB));

		_shadowTechnique = std::make_shared<CascadedPCFShadow>();
		_shadowTechnique->Cast<CascadedPCFShadow>()->SetFilterSize(3.0f);
		_shadowTechnique->SetShadowMapSize(1024);
	}

	void DirectionalLightComponent::SetInfluenceAll(bool bInfluenceAll)
	{
		if (_bInfluenceAll != bInfluenceAll && IsActive())
			Global::GetRenderEngine()->GetSceneRenderLightsCuller()->UpdateElement(shared_from_this()->Cast<LightComponent>());

		_bInfluenceAll = bInfluenceAll;
		if (_bInfluenceAll)
			memset(&_influenceAABB, 0, sizeof(_influenceAABB));
	}

	AABBox DirectionalLightComponent::GetBoundsAABB() const
	{
		if (_bInfluenceAll)
			return AABBox(-FLT_MAX, FLT_MAX);
		else
			return _influenceAABB;
	}

	void DirectionalLightComponent::BindMacros(bool enableShadow, const Ptr<RenderView> & view, std::map<String, String> & outMacros)
	{
		LightComponent::BindMacros(enableShadow, view, outMacros);

		outMacros["LIGHT_TYPE"] = "LIGHT_TYPE_DIRECTIONAL";
	}

	void DirectionalLightComponent::BindShaderParams(const Ptr<Shader> & shader, bool enableShadow, const Ptr<RenderView> & view)
	{
		LightComponent::BindShaderParams(shader, enableShadow, view);

		shader->SetScalar("lightDir", _direction);
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

	float2 DirectionalLightComponent::GetClipSpacePos(const Ptr<Camera> & camera) const
	{
		auto & cameraPos = camera->GetPos();
		float dist = GetDistance();
		auto posW = cameraPos - Direction() * dist;/*float3(
			cameraPos.x - Direction().x * dist,
			cameraPos.y - Direction().y * dist,
			cameraPos.z - Direction().z * dist);*/
		/*auto posWXM = XMLoadFloat3(&posW);
		auto viewXM = XMLoadFloat4x4(&camera->GetViewMatrix());
		auto projXM = XMLoadFloat4x4(&camera->GetProjMatrix());
		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
		auto posHXM = XMVector3TransformCoord(posWXM, viewProjXM);*/
		auto posH = transform_coord(posW, camera->GetViewProjMatrix());
		/*XMStoreFloat4(&posH, posHXM);
		posH.x /= posH.w;
		posH.y /= posH.w;*/

		return float2(posH.x(), posH.y());
	}
}