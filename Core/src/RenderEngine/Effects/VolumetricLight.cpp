#include "ToyGE\RenderEngine\Effects\VolumetricLight.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	VolumetricLight::VolumetricLight()
		: _attenuation(0.1f),
		_scattering(0.5f),
		_phaseFunctionParam(0.5f)
	{
		auto pointLightVolumeMesh = CommonMesh::CreateSphere(1.0f, 25);
		_pointLightVolumeGeo = std::make_shared<RenderMeshComponent>();
		_pointLightVolumeGeo->SetMesh(pointLightVolumeMesh);

		auto spotLightVolumeMesh = CommonMesh::CreateCone(1.0f, XM_PIDIV2, 50);
		_spotLightVolumeGeo = std::make_shared<RenderMeshComponent>();
		_spotLightVolumeGeo->SetMesh(spotLightVolumeMesh);

		InitDither();
	}

	void VolumetricLight::Render(const Ptr<RenderView> & view)
	{
		bool bNeedRender = false;
		for (auto & light : view->GetViewRenderContext()->lights)
		{
			if (light->IsCastLightVolume())
			{
				bNeedRender = true;
				break;
			}
		}
		if (!bNeedRender)
			return;

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");

		auto lowResLinearDepthDesc = sceneLinearClipDepth->GetDesc();
		lowResLinearDepthDesc.width /= 2;
		lowResLinearDepthDesc.height /= 2;
		auto lowResLinearDepthRef = TexturePool::Instance().FindFree({ TEXTURE_2D, lowResLinearDepthDesc });
		auto lowResLinearDepth = lowResLinearDepthRef->Get()->Cast<Texture>();
		Transform(sceneLinearClipDepth->GetShaderResourceView(), lowResLinearDepth->GetRenderTargetView(0, 0, 1));

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto lowResLightVolumeDesc = sceneTex->GetDesc();
		lowResLightVolumeDesc.width /= 2;
		lowResLightVolumeDesc.height /= 2;
		auto lowResLightVolumeRef = TexturePool::Instance().FindFree({ TEXTURE_2D, lowResLightVolumeDesc });
		auto lowResLightVolume = lowResLightVolumeRef->Get()->Cast<Texture>();
		rc->ClearRenderTarget(lowResLightVolume->GetRenderTargetView(0, 0, 1), 0.0f);

		rc->SetViewport(GetTextureQuadViewport(lowResLightVolume));

		for (auto & light : view->GetViewRenderContext()->lights)
		{
			if (light->IsCastLightVolume() && light->IsCastShadow())
			{
				switch (light->Type())
				{
				case LIGHT_POINT:
				{
					auto pointLight = std::static_pointer_cast<PointLightComponent>(light);
					RenderPointLightVolume(pointLight, view, lowResLinearDepth, lowResLightVolume);
					break;
				}

				case LIGHT_SPOT:
				{
					auto spotLight = std::static_pointer_cast<SpotLightComponent>(light);
					RenderSpotLightVolume(spotLight, view, lowResLinearDepth, lowResLightVolume);
					break;
				}

				case LIGHT_DIRECTIONAL:
				{
					auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
					RenderDirectionalLightVolume(dirLight, view, lowResLinearDepth, lowResLightVolume);
					break;
				}

				default:
					break;
				}
			}
		}

		float depthDiffThreshold = 0.1f / (view->GetCamera()->GetFar() - view->GetCamera()->GetNear());
		//Blur
		//int32_t blurRadius = 2;
		////auto lightVolumeTexBlur = BilateralGaussBlur(lightVolumeTex, lowResLinearDepthTex);
		//BilateralBlur(
		//	lowResLightVolume->GetShaderResourceView(), 
		//	lowResLightVolume->GetRenderTargetView(0, 0, 1), 
		//	lowResLinearDepth->GetShaderResourceView(), 
		//	GetGaussTable(blurRadius * 2 + 1), 
		//	depthDiffThreshold);

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		BilateralUpSampling(
			lowResLightVolume->GetShaderResourceView(), 
			sceneTex->GetRenderTargetView(0, 0, 1), 
			lowResLinearDepth->GetShaderResourceView(), 
			sceneLinearClipDepth->GetShaderResourceView(), 
			depthDiffThreshold);

		rc->SetBlendState(nullptr);
	}

	void VolumetricLight::InitDither()
	{
		float data[] = 
		{
			0.0f, 8.0f, 2.0f, 10.0f,
			12.0f, 4.0f, 14.0f, 6.0f,
			3.0f, 11.0f, 1.0f, 9.0f,
			15.0f, 7.0f, 13.0f, 5.0f
		};
		for (auto & i : data)
		{
			i /= 16.0f;
		}

		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE;
		bufDesc.bStructured = false;
		bufDesc.cpuAccess = 0;
		bufDesc.elementSize = sizeof(float);
		bufDesc.numElements = 16;

		_ditherBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		_ditherBuffer->SetDesc(bufDesc);
		_ditherBuffer->Init(data);
	}

	void VolumetricLight::RenderPointLightVolume(
		const Ptr<PointLightComponent> & light,
		const Ptr<RenderView> & view,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		auto meshElement = _pointLightVolumeGeo->GetSubRenderComponents()[0]->GetMeshElement();

		std::map<String, String> macros;
		meshElement->BindMacros(macros);
		light->BindMacros(true, view, macros);

		auto vs = Shader::FindOrCreate<RenderLightVolumeVS>(macros);
		auto ps = Shader::FindOrCreate<RenderPointLightVolumePS>(macros);

		auto & lightPos = light->GetPos();
		_pointLightVolumeGeo->SetPos(lightPos);
		float maxDist = light->MaxDistance();
		_pointLightVolumeGeo->SetScale(XMFLOAT3(maxDist, maxDist, maxDist));
		_pointLightVolumeGeo->UpdateTransform();
		_pointLightVolumeGeo->BindShaderParams(vs);

		view->BindShaderParams(vs);
		view->BindShaderParams(ps);

		light->BindShaderParams(ps, true, view);

		ps->SetScalar("pointLightRadius", maxDist);
		ps->SetScalar("attenuation", _attenuation);
		ps->SetScalar("scattering", _scattering);
		ps->SetScalar("phaseFunctionParam", _phaseFunctionParam);
		ps->SetScalar("targetSize", lightVolumeTex->GetTexSize());

		ps->SetSRV("sceneLinearClipDepth", linearDepthTex->GetShaderResourceView());

		vs->Flush();
		ps->Flush();

		auto stencilDSDesc = lightVolumeTex->GetDesc();
		stencilDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		stencilDSDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		auto stencilDSRef = TexturePool::Instance().FindFree({ TEXTURE_2D, stencilDSDesc });
		stencilDSRef->Get()->Cast<Texture>()->SetDesc(stencilDSDesc);

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencil(stencilDSRef->Get()->Cast<Texture>()->GetDepthStencilView(0, 0, 1));
		Global::GetRenderEngine()->GetRenderContext()->ClearDepthStencil(1.0f, 0);

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ lightVolumeTex->GetRenderTargetView(0, 0, 1) });

		auto dss = DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true,
			0xff,
			0xff,
			STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_INCR, COMPARISON_EQUAL,
			STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_INCR, COMPARISON_EQUAL>::Get();

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencilState(dss, 0);
		Global::GetRenderEngine()->GetRenderContext()->SetRasterizerState(RasterizerStateTemplate<FILL_SOLID, CULL_NONE, false, 0, false>::Get());
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		meshElement->Draw();

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencilState(nullptr);
		Global::GetRenderEngine()->GetRenderContext()->SetRasterizerState(nullptr);
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}

	void VolumetricLight::RenderSpotLightVolume(
		const Ptr<SpotLightComponent> & light,
		const Ptr<RenderView> & view,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		auto meshElement = _pointLightVolumeGeo->GetSubRenderComponents()[0]->GetMeshElement();

		std::map<String, String> macros;
		meshElement->BindMacros(macros);
		light->BindMacros(true, view, macros);

		auto vs = Shader::FindOrCreate<RenderLightVolumeVS>(macros);
		auto ps = Shader::FindOrCreate<RenderSpotLightVolumePS>(macros);

		auto & lightPos = light->GetPos();
		float maxDist = light->MaxDistance();
		float3 lightPosf3 = *(reinterpret_cast<const float3*>(&lightPos));
		float3 lightDirf3 = *(reinterpret_cast<const float3*>(&light->Direction()));
		float3 geoPos = lightPosf3 + maxDist * lightDirf3;
		_spotLightVolumeGeo->SetPos(XMFLOAT3(geoPos.x(), geoPos.y(), geoPos.z()));

		float angle = light->MaxAngle();
		float xzScale = tan(angle) * maxDist;
		_spotLightVolumeGeo->SetScale(XMFLOAT3(xzScale, maxDist, xzScale));

		float3 v0 = float3(0.0f, -1.0f, 0.0f);
		float3 v1 = *(reinterpret_cast<const float3*>(&light->Direction()));
		float3 rotateAxis = cross(v0, v1);
		float sinAngle = length(rotateAxis);
		float rotateAngle = std::asin(sinAngle);
		float sinAngle_d2 = sin(rotateAngle * 0.5f);
		float cosAngle_d2 = cos(rotateAngle * 0.5f);
		rotateAxis = normalize(rotateAxis);

		float4 orientation = float4(
			rotateAxis.x() * sinAngle_d2,
			rotateAxis.y() * sinAngle_d2,
			rotateAxis.z() * sinAngle_d2,
			cosAngle_d2);
		_spotLightVolumeGeo->SetOrientation(XMFLOAT4(orientation.x(), orientation.y(), orientation.z(), orientation.w()));
		_spotLightVolumeGeo->UpdateTransform();
		_spotLightVolumeGeo->BindShaderParams(vs);

		view->BindShaderParams(vs);
		view->BindShaderParams(ps);

		light->BindShaderParams(ps, true, view);

		ps->SetScalar("spotLightAngle", angle);
		//ps->SetScalar("spotLightDecrease", light->DecreaseSpeed());
		ps->SetScalar("attenuation", _attenuation);
		ps->SetScalar("scattering", _scattering);
		ps->SetScalar("phaseFunctionParam", _phaseFunctionParam);
		ps->SetScalar("targetSize", lightVolumeTex->GetTexSize());

		ps->SetSRV("sceneLinearClipDepth", linearDepthTex->GetShaderResourceView());

		vs->Flush();
		ps->Flush();

		auto stencilDSDesc = lightVolumeTex->GetDesc();
		stencilDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		stencilDSDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		auto stencilDSRef = TexturePool::Instance().FindFree({ TEXTURE_2D, stencilDSDesc });
		stencilDSRef->Get()->Cast<Texture>()->SetDesc(stencilDSDesc);

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencil(stencilDSRef->Get()->Cast<Texture>()->GetDepthStencilView(0, 0, 1));
		Global::GetRenderEngine()->GetRenderContext()->ClearDepthStencil(1.0f, 0);

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ lightVolumeTex->GetRenderTargetView(0, 0, 1) });

		auto dss = DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true,
			0xff,
			0xff,
			STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_INCR, COMPARISON_EQUAL,
			STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_INCR, COMPARISON_EQUAL>::Get();

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencilState(dss, 0);
		Global::GetRenderEngine()->GetRenderContext()->SetRasterizerState(RasterizerStateTemplate<FILL_SOLID, CULL_NONE, false, 0, false>::Get());
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		meshElement->Draw();

		Global::GetRenderEngine()->GetRenderContext()->SetDepthStencilState(nullptr);
		Global::GetRenderEngine()->GetRenderContext()->SetRasterizerState(nullptr);
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}

	void VolumetricLight::RenderDirectionalLightVolume(
		const Ptr<DirectionalLightComponent> & light,
		const Ptr<RenderView> & view,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		std::map<String, String> macros;
		light->BindMacros(true, view, macros);

		auto ps = Shader::FindOrCreate<RenderDirectionalLightVolumePS>(macros);

		view->BindShaderParams(ps);
		light->BindShaderParams(ps, true, view);

		float dirLightDist = 10.0f;
		ps->SetScalar("dirLightDist", dirLightDist);
		ps->SetScalar("attenuation", _attenuation);
		ps->SetScalar("scattering", _scattering);
		ps->SetScalar("phaseFunctionParam", _phaseFunctionParam);
		ps->SetScalar("targetSize", lightVolumeTex->GetTexSize());

		ps->SetSRV("sceneLinearClipDepth", linearDepthTex->GetShaderResourceView());

		ps->Flush();

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		DrawQuad({ lightVolumeTex->GetRenderTargetView(0, 0, 1) });

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}
}