#include "ToyGE\RenderEngine\Effects\VolumetricLight.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	VolumetricLight::VolumetricLight()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"VolumetricLight.xml");

		auto sphereMesh = CommonMesh::CreateSphere(1.0f, 50);
		sphereMesh->InitRenderData();
		_pointLightVolumeGeo = std::make_shared<RenderComponent>();
		_pointLightVolumeGeo->SetMesh(sphereMesh);

		auto coneMesh = CommonMesh::CreateCone(1.0f, XM_PIDIV2, 50);
		coneMesh->InitRenderData();
		_spotLightVolumeGeo = std::make_shared<RenderComponent>();
		_spotLightVolumeGeo->SetMesh(coneMesh);

		InitDither();
	}

	void VolumetricLight::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		bool bNeedRender = false;
		for (auto & light : sharedEnviroment->GetView()->GetRenderLights())
		{
			if (light->IsCastLightVolume())
			{
				bNeedRender = true;
				break;
			}
		}
		if (!bNeedRender)
			return;

		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();//Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);

		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto lowResLinearDepthTex = DownSample(linearDepthTex->CreateTextureView(), 0.5f);

		//Set Light Volume Rendering Params
		/*auto camera = sharedEnviroment->GetView()->GetCamera();
		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		auto viewXM = XMLoadFloat4x4(&camera->ViewMatrix());
		auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
		XMFLOAT4X4 invView;
		XMStoreFloat4x4(&invView, invViewXM);
		_fx->VariableByName("invView")->AsScalar()->SetValue(&invView);
		_fx->VariableByName("cameraPos")->AsScalar()->SetValue(&camera->Pos());
		float2 cameraNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);*/
		sharedEnviroment->GetView()->BindParams(_fx);

		//Render in Low Res
		auto rc = Global::GetRenderEngine()->GetRenderContext();
		auto preVP = rc->GetViewport();
		auto lowResVP = preVP;
		lowResVP.width = static_cast<float>(sceneTex->Desc().width / 2);
		lowResVP.height = static_cast<float>(sceneTex->Desc().height / 2);
		rc->SetViewport(lowResVP);

		//Render Light Volume
		TextureDesc texDesc = sceneTex->Desc();
		texDesc.width /= 2;
		texDesc.height /= 2;
		auto lightVolumeTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		rc->ClearRenderTargets({ lightVolumeTex->CreateTextureView() }, 0.0f);

		for (auto & light : sharedEnviroment->GetView()->GetRenderLights())
		{
			if (light->IsCastLightVolume())
			{
				switch (light->Type())
				{
				case LIGHT_POINT:
				{
					auto pointLight = std::static_pointer_cast<PointLightComponent>(light);
					RenderPointLightVolume(pointLight, sharedEnviroment->GetView()->GetCamera(), lowResLinearDepthTex, lightVolumeTex);
					break;
				}

				case LIGHT_SPOT:
				{
					auto spotLight = std::static_pointer_cast<SpotLightComponent>(light);
					RenderSpotLightVolume(spotLight, sharedEnviroment->GetView()->GetCamera(), lowResLinearDepthTex, lightVolumeTex);
					break;
				}

				case LIGHT_DIRECTIONAL:
				{
					auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
					RenderDirectionalLightVolume(dirLight, sharedEnviroment->GetView()->GetCamera(), lowResLinearDepthTex, lightVolumeTex);
					break;
				}

				default:
					break;
				}
			}
		}

		//Blur
		auto lightVolumeTexBlur = BilateralGaussBlur(lightVolumeTex, lowResLinearDepthTex);

		//Upsampling
		BilateralUpSampling(lightVolumeTexBlur, linearDepthTex, lowResLinearDepthTex, sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();

		//Restore Viewport
		rc->SetViewport(preVP);

		//sceneTex->Release();
		lowResLinearDepthTex->Release();
		lightVolumeTex->Release();
		lightVolumeTexBlur->Release();
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

		_fx->VariableByName("dither")->AsScalar()->SetValue(data);
	}

	void VolumetricLight::RenderPointLightVolume(
		const Ptr<PointLightComponent> & light,
		const Ptr<Camera> & camera,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto & lightPos = light->GetPos();
		_pointLightVolumeGeo->SetPos(lightPos);
		float maxDist = light->MaxDistance();
		_pointLightVolumeGeo->SetScale(XMFLOAT3(maxDist, maxDist, maxDist));
		_pointLightVolumeGeo->UpdateTransform();
		_fx->VariableByName("world")->AsScalar()->SetValue(&_pointLightVolumeGeo->GetTransformMatrix());

		/*_fx->VariableByName("lightPos")->AsScalar()->SetValue(&lightPos);
		_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&light->Radiance());*/
		_fx->VariableByName("pointLightRadius")->AsScalar()->SetValue(&maxDist);

		light->BindParams(_fx, false, camera);

		float2 texelSize = 1.0f / float2(static_cast<float>(lightVolumeTex->Desc().width), static_cast<float>(lightVolumeTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		float attenuation = 0.1f;
		float scattering = 10.0f;
		float phaseFunctionParam = 0.5f;
		_fx->VariableByName("attenuation")->AsScalar()->SetValue(&attenuation);
		_fx->VariableByName("scattering")->AsScalar()->SetValue(&scattering);
		_fx->VariableByName("phaseFunctionParam")->AsScalar()->SetValue(&phaseFunctionParam);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		//int4 shadowConfig = 0;
		//if (light->IsCastShadow())
		//{
		//	shadowConfig.x = 1;
		//	shadowConfig.y = light->GetShadowTechnique()->RenderTechnique()->Type();
		//	light->GetShadowTechnique()->BindShadowRenderParams(_fx, light, camera);
		//}
		//_fx->VariableByName("shadowConfig")->AsScalar()->SetValue(&shadowConfig, sizeof(shadowConfig));

		auto tmpDSDesc = lightVolumeTex->Desc();
		tmpDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		tmpDSDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		auto tmpDS = re->GetRenderFactory()->GetTexturePooled(tmpDSDesc);
		rc->SetDepthStencil(tmpDS->CreateTextureView());
		rc->ClearDepthStencil(1.0f, 0);

		rc->SetRenderTargets({ lightVolumeTex->CreateTextureView() }, 0);
		rc->SetRenderInput(_pointLightVolumeGeo->GetMesh()->AcquireRender()->GetRenderInput());

		_fx->TechniqueByName("RenderLightVolumePoint")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("RenderLightVolumePoint")->PassByIndex(0)->UnBind();

		tmpDS->Release();
	}

	void VolumetricLight::RenderSpotLightVolume(
		const Ptr<SpotLightComponent> & light,
		const Ptr<Camera> & camera,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto & lightPos = light->GetPos();
		float maxDist = light->MaxDistance();
		float3 lightPosf3 = *(reinterpret_cast<const float3*>(&lightPos));
		float3 lightDirf3 = *(reinterpret_cast<const float3*>(&light->Direction()));
		float3 geoPos = lightPosf3 + maxDist * lightDirf3;
		_spotLightVolumeGeo->SetPos(XMFLOAT3(geoPos.x, geoPos.y, geoPos.z));

		float angle = light->MaxAngle();
		float xzScale = tan(angle) * maxDist;
		_spotLightVolumeGeo->SetScale(XMFLOAT3(xzScale, maxDist, xzScale));

		float3 v0 = float3(0.0f, 1.0f, 0.0f);
		float3 v1 = *(reinterpret_cast<const float3*>(&light->Direction()));
		float3 rotateAxis = cross(v0, v1);
		float sinAngle = length(rotateAxis);
		//rotateAxis /= sinAngle;
		float4 orientation = float4(rotateAxis.x, rotateAxis.y, rotateAxis.z, sqrt(1.0f - sinAngle * sinAngle));
		_spotLightVolumeGeo->SetOrientation(XMFLOAT4(orientation.x, orientation.y, orientation.z, orientation.w));
		_spotLightVolumeGeo->UpdateTransform();
		_fx->VariableByName("world")->AsScalar()->SetValue(&_spotLightVolumeGeo->GetTransformMatrix());


		//_fx->VariableByName("lightPos")->AsScalar()->SetValue(&lightPos);
		//_fx->VariableByName("lightDir")->AsScalar()->SetValue(&light->Direction());
		//_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&light->Radiance());
		_fx->VariableByName("spotLightAngle")->AsScalar()->SetValue(&angle);
		_fx->VariableByName("spotLightDecrease")->AsScalar()->SetValue(&light->DecreaseSpeed());

		light->BindParams(_fx, false, camera);

		float2 texelSize = 1.0f / float2(static_cast<float>(lightVolumeTex->Desc().width), static_cast<float>(lightVolumeTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		float attenuation = 0.1f;
		float scattering = 10.0f;
		float phaseFunctionParam = 0.5f;
		_fx->VariableByName("attenuation")->AsScalar()->SetValue(&attenuation);
		_fx->VariableByName("scattering")->AsScalar()->SetValue(&scattering);
		_fx->VariableByName("phaseFunctionParam")->AsScalar()->SetValue(&phaseFunctionParam);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		//int4 shadowConfig = 0;
		//if (light->IsCastShadow())
		//{
		//	shadowConfig.x = 1;
		//	shadowConfig.y = light->GetShadowTechnique()->RenderTechnique()->Type();
		//	light->GetShadowTechnique()->BindShadowRenderParams(_fx, light, camera);
		//}
		//_fx->VariableByName("shadowConfig")->AsScalar()->SetValue(&shadowConfig, sizeof(shadowConfig));

		auto tmpDSDesc = lightVolumeTex->Desc();
		tmpDSDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		tmpDSDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		auto tmpDS = re->GetRenderFactory()->GetTexturePooled(tmpDSDesc);
		rc->SetDepthStencil(tmpDS->CreateTextureView());
		rc->ClearDepthStencil(1.0f, 0);

		rc->SetRenderTargets({ lightVolumeTex->CreateTextureView() }, 0);
		rc->SetRenderInput(_spotLightVolumeGeo->GetMesh()->AcquireRender()->GetRenderInput());

		_fx->TechniqueByName("RenderLightVolumeSpot")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("RenderLightVolumeSpot")->PassByIndex(0)->UnBind();

		tmpDS->Release();
	}

	void VolumetricLight::RenderDirectionalLightVolume(
		const Ptr<DirectionalLightComponent> & light,
		const Ptr<Camera> & camera,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & lightVolumeTex)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		float dirLightDist = 10.0f;
		_fx->VariableByName("dirLightDist")->AsScalar()->SetValue(&dirLightDist);

		/*_fx->VariableByName("lightDir")->AsScalar()->SetValue(&light->Direction());
		_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&light->Radiance());*/

		light->BindParams(_fx, false, camera);

		float2 texelSize = 1.0f / float2(static_cast<float>(lightVolumeTex->Desc().width), static_cast<float>(lightVolumeTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		float attenuation = 0.1f;
		float scattering = 10.0f;
		float phaseFunctionParam = 0.5f;
		_fx->VariableByName("attenuation")->AsScalar()->SetValue(&attenuation);
		_fx->VariableByName("scattering")->AsScalar()->SetValue(&scattering);
		_fx->VariableByName("phaseFunctionParam")->AsScalar()->SetValue(&phaseFunctionParam);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		//int4 shadowConfig = 0;
		//if (light->IsCastShadow())
		//{
		//	shadowConfig.x = 1;
		//	shadowConfig.y = light->GetShadowTechnique()->RenderTechnique()->Type();
		//	light->GetShadowTechnique()->BindShadowRenderParams(_fx, light, camera);
		//}
		//_fx->VariableByName("shadowConfig")->AsScalar()->SetValue(&shadowConfig, sizeof(shadowConfig));

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetRenderTargets({ lightVolumeTex->CreateTextureView() }, 0);
		rc->SetDepthStencil(ResourceView());

		_fx->TechniqueByName("RenderLightVolumeDirectional")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("RenderLightVolumeDirectional")->PassByIndex(0)->UnBind();
	}

	Ptr<Texture> VolumetricLight::BilateralGaussBlur(const Ptr<Texture> & lightVolumeTex, const Ptr<Texture> & linearDepthTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = lightVolumeTex->Desc();
		auto resultTexTmp = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		int blurRadius = 3;
		auto & gaussTable = Blur::GaussTable(blurRadius);
		_fx->VariableByName("gaussTable")->AsScalar()->SetValue(&gaussTable[0], sizeof(float) * gaussTable.size());

		float2 texelSize = 1.0f / float2(static_cast<float>(texDesc.width), static_cast<float>(texDesc.height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		//BlurX
		_fx->VariableByName("lightVolumeTex")->AsShaderResource()->SetValue(lightVolumeTex->CreateTextureView());
		rc->SetRenderTargets({ resultTexTmp->CreateTextureView() }, 0);
		_fx->TechniqueByName("BilateralGaussBlurX")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("BilateralGaussBlurX")->PassByIndex(0)->UnBind();

		//BlurY
		_fx->VariableByName("lightVolumeTex")->AsShaderResource()->SetValue(resultTexTmp->CreateTextureView());
		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		_fx->TechniqueByName("BilateralGaussBlurY")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("BilateralGaussBlurY")->PassByIndex(0)->UnBind();

		resultTexTmp->Release();

		return resultTex;
	}

	void VolumetricLight::BilateralUpSampling(
		const Ptr<Texture> & lightVolumeTex,
		const Ptr<Texture> & highResLinearDepthTex,
		const Ptr<Texture> & lowResLinearDepthTex,
		const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderViewport vp;
		vp.width = static_cast<float>(highResLinearDepthTex->Desc().width);
		vp.height = static_cast<float>(highResLinearDepthTex->Desc().height);
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		float2 texelSize = 1.0f / float2(vp.width, vp.height);
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("lightVolumeTex")->AsShaderResource()->SetValue(lightVolumeTex->CreateTextureView());
		_fx->VariableByName("highResLinearDepthTex")->AsShaderResource()->SetValue(highResLinearDepthTex->CreateTextureView());
		_fx->VariableByName("lowResLinearDepthTex")->AsShaderResource()->SetValue(lowResLinearDepthTex->CreateTextureView());

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetRenderTargets({ target }, 0);
		rc->SetDepthStencil(ResourceView());

		_fx->TechniqueByName("BilateralUpSampling")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("BilateralUpSampling")->PassByIndex(0)->UnBind();
	}
}