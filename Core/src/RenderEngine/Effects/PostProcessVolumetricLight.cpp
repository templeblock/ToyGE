//#include "ToyGE\RenderEngine\Effects\PostProcessVolumetricLight.h"
//#include "ToyGE\Kernel\Global.h"
//#include "ToyGE\Kernel\ResourceManager.h"
//#include "ToyGE\RenderEngine\RenderEffect.h"
//#include "ToyGE\RenderEngine\RenderEffectVariable.h"
//#include "ToyGE\RenderEngine\RenderTechnique.h"
//#include "ToyGE\RenderEngine\RenderPass.h"
//#include "ToyGE\RenderEngine\RenderEngine.h"
//#include "ToyGE\RenderEngine\RenderContext.h"
//#include "ToyGE\RenderEngine\Texture.h"
//#include "ToyGE\RenderEngine\RenderFactory.h"
//#include "ToyGE\RenderEngine\SceneManager.h"
//#include "ToyGE\RenderEngine\LightComponent.h"
//#include "ToyGE\RenderEngine\Camera.h"
//#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
//#include "ToyGE\RenderEngine\RenderInput.h"
//
//namespace ToyGE
//{
//	PostProcessVolumetricLight::PostProcessVolumetricLight()
//	{
//		_volumetricLightFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource("PostProcessVolumetricLight.xml");
//	}
//
//	Ptr<Texture> PostProcessVolumetricLight::Process(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
//	{
//		if (!_inTex)
//			return _inTex;
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//
//		RenderContextStateSave stateSave;
//		rc->SaveState(
//			RENDER_CONTEXT_STATE_INPUT
//			| RENDER_CONTEXT_STATE_VIEWPORT
//			| RENDER_CONTEXT_STATE_DEPTHSTENCIL
//			| RENDER_CONTEXT_STATE_RENDERTARGETS, stateSave);
//
//		rc->SetDepthStencil(ResourceView());
//		rc->SetRenderInput(CommonInput::QuadInput());
//
//		auto light = std::static_pointer_cast<DirectionalLightComponent>( Global::GetRenderEngine()->SceneManager()->GetLight(0) );
//		auto & dir = light->Direction();
//		float3 pos = float3(dir.x, dir.y, dir.z) * -50.0f;
//		auto lightPosXM = XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&pos));
//		auto camera = Global::GetRenderEngine()->Camera();
//		auto viewXM = XMLoadFloat4x4(&camera->ViewMatrix());
//		auto projXM = XMLoadFloat4x4(&camera->ProjMatrix());
//		auto viewProjXM = XMMatrixMultiply(viewXM, projXM);
//		auto lightPosProjXM = XMVector3TransformCoord(lightPosXM, viewProjXM);
//		float3 lightPosH;
//		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&lightPosH), lightPosProjXM);
//		lightPosH = vecMax(vecMin(lightPosH, float3(1.0f)), float3(-1.0f));
//		lightPosH.x = lightPosH.x * 0.5f + 0.5f;
//		lightPosH.y = lightPosH.y * -0.5f + 0.5f;
//
//		auto texDesc = _inTex->Desc();
//		auto prePassTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
//		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
//		rc->ClearRenderTargets({ resultTex->CreateTextureView() }, { 0.0f, 0.0f, 0.0f, 0.0f });
//		auto rawDepthTex = sharedEnviroment->ParamByName("rawDepthTex")->As<SharedParamTexture>()->Texture();
//		auto depthTex = sharedEnviroment->ParamByName("depthLinearTex")->As<SharedParamTexture>()->Texture();
//
//		_volumetricLightFX->VariableByName("sceneTex")->AsShaderResource()->SetValue(_inTex->CreateTextureView());
//		_volumetricLightFX->VariableByName("depthTex")->AsShaderResource()->SetValue(depthTex->CreateTextureView());
//		rc->SetRenderTargets({ prePassTex->CreateTextureView() }, 0);
//		_volumetricLightFX->TechniqueByName("VolumetricLight_PrePass")->PassByIndex(0)->Bind();
//		rc->DrawIndexed();
//		_volumetricLightFX->TechniqueByName("VolumetricLight_PrePass")->PassByIndex(0)->UnBind();
//
//		_volumetricLightFX->VariableByName("prePassTex")->AsShaderResource()->SetValue(prePassTex->CreateTextureView());
//		rc->SetRenderTargets({ _inTex->CreateTextureView() }, 0);
//		_volumetricLightFX->VariableByName("screenLightPos")->AsScalar()->SetValue(&lightPosH, sizeof(lightPosH));
//		_volumetricLightFX->TechniqueByName("VolumetricLight")->PassByIndex(0)->Bind();
//		rc->DrawIndexed();
//		_volumetricLightFX->TechniqueByName("VolumetricLight")->PassByIndex(0)->UnBind();
//
//		//_volumetricLightFX->VariableByName("colorDepthTex")->AsShaderResource()->SetValue(_inTex->CreateTextureView());
//		//rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
//		//_volumetricLightFX->TechniqueByName("VolumetricLight")->PassByIndex(0)->Bind();
//		//rc->DrawIndexed();
//		//_volumetricLightFX->TechniqueByName("VolumetricLight")->PassByIndex(0)->UnBind();
//
//
//
//
//		rc->RestoreState(stateSave);
//
//		return _inTex;
//	}
//}