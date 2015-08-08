#include "ToyGE\RenderEngine\Effects\DeferredRendering.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	DeferredRenderingBase::DeferredRenderingBase()
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DeferredRenderingBase.xml");
		_normalsFittingTex = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>()->AcquireResource(L"normalsFitting.dds");
	}

	void DeferredRenderingBase::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto factory = re->GetRenderFactory();

		//Init Textures
		TextureDesc desc;
		desc.width = static_cast<int32_t>(sharedEnviroment->GetView()->GetViewport().width);
		desc.height = static_cast<int32_t>(sharedEnviroment->GetView()->GetViewport().height);
		desc.depth = 1;
		desc.arraySize = 1;
		desc.cpuAccess = 0;
		desc.mipLevels = 1;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.type = TEXTURE_2D;

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		desc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		auto gbuffer0 = factory->GetTexturePooled(desc);
		auto gbuffer1 = factory->GetTexturePooled(desc);

		desc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL | TEXTURE_BIND_SHADER_RESOURCE;
		desc.format = RENDER_FORMAT_R24G8_TYPELESS;
		auto rawDepthTex = factory->GetTexturePooled(desc);

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		desc.format = RENDER_FORMAT_R32_FLOAT;
		auto linearDepth = factory->GetTexturePooled(desc);

		desc.format = RENDER_FORMAT_R32_SINT;
		auto reflectionMapID = factory->GetTexturePooled(desc);

		Ptr<Texture> velocity;
		if (CheckRenderConfig<String>(sharedEnviroment, "GenMotionVelocityMap", "true"))
		{
			desc.format = RENDER_FORMAT_R16G16_FLOAT;
			velocity = factory->GetTexturePooled(desc);
		}
		
		//Clear
		rc->ClearRenderTargets(
		{
			gbuffer0->CreateTextureView(),
			gbuffer1->CreateTextureView(),
			reflectionMapID->CreateTextureView()
		},
		0.0f);

		rc->ClearRenderTargets(
		{
			linearDepth->CreateTextureView(),
		},
		1.0f);

		rc->ClearDepthStencil(rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT), 1.0f, 0);
		
		if (velocity)
		{
			rc->ClearRenderTargets({ velocity->CreateTextureView() }, 0.0f);
			rc->SetRenderTargets(
			{
				gbuffer0->CreateTextureView(),
				gbuffer1->CreateTextureView(),
				linearDepth->CreateTextureView(),
				reflectionMapID->CreateTextureView(),
				velocity->CreateTextureView()
			}, 0);
		}
		else
		{
			rc->SetRenderTargets(
			{
				gbuffer0->CreateTextureView(),
				gbuffer1->CreateTextureView(),
				linearDepth->CreateTextureView(),
				reflectionMapID->CreateTextureView()
			}, 0);
		}

		rc->SetDepthStencil(rawDepthTex->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		sharedEnviroment->GetView()->BindParams(_fx);
		_fx->VariableByName("normalFittingTex")->AsShaderResource()->SetValue(_normalsFittingTex->CreateTextureView());

		//Sort Objects
		std::vector<Ptr<RenderComponent>> sortedRenderObjs = sharedEnviroment->GetView()->GetRenderObjects();
		std::sort(sortedRenderObjs.begin(), sortedRenderObjs.end(),
			[&](const Ptr<RenderComponent> & obj0, const Ptr<RenderComponent> & obj1) -> bool
		{
			return (obj0->GetMaterial()->GetTypeFlags()) < (obj0->GetMaterial()->GetTypeFlags());
		});
		std::stable_sort(sortedRenderObjs.begin(), sortedRenderObjs.end(),
			[](const Ptr<RenderComponent> & obj0, const Ptr<RenderComponent> & obj1) -> bool
		{
			return obj0->GetMaterial() < obj0->GetMaterial();
		});

		if (velocity)
			_fx->AddExtraMacro("GEN_VELOCITY", "" );
		else
			_fx->RemoveExtraMacro("GEN_VELOCITY");


		//Render Each Opaque Object
		Ptr<Material> curMat;
		for (auto & obj : sortedRenderObjs)
		{
			if (obj->GetMaterial()->IsTranslucent())
				continue;

			//Update Shader Material Params
			if (curMat != obj->GetMaterial())
			{
				curMat = obj->GetMaterial();
				curMat->BindMacros(_fx);
				_fx->UpdateData();
				curMat->BindParams(_fx);

				if (curMat->IsSubSurfaceScattering())
					_fx->TechniqueByName("GBuffer")->PassByIndex(0)->SetStencilRef(2);
				else
					_fx->TechniqueByName("GBuffer")->PassByIndex(0)->SetStencilRef(1);
			}

			int32_t reflectionMapID = -1;
			if (obj->GetReflectionMap())
			{
				reflectionMapID = obj->GetReflectionMap()->GetID();
			}
			_fx->VariableByName("reflectionMapID")->AsScalar()->SetValue(&reflectionMapID, sizeof(reflectionMapID));

			//Render Object
			rc->SetRenderInput(obj->GetMesh()->AcquireRender()->GetRenderInput());
			auto technique = _fx->TechniqueByName("GBuffer");
			_fx->VariableByName("world")->AsScalar()->SetValue(&obj->GetTransformMatrix());
			_fx->VariableByName("preWorld")->AsScalar()->SetValue(&obj->GetTransformMatrixCache());
			for (int32_t passIndex = 0; passIndex < technique->NumPasses(); ++passIndex)
			{
				technique->PassByIndex(passIndex)->Bind();
				rc->DrawIndexed();
				technique->PassByIndex(passIndex)->UnBind();
			}
			obj->SetTransformMatrixCache(obj->GetTransformMatrix());
		}
		sharedEnviroment->GetView()->GetCamera()->SetViewMatrixCache(sharedEnviroment->GetView()->GetCamera()->ViewMatrix());

		rc->SetDepthStencil(ResourceView());

		//Set Share
		sharedEnviroment->SetParam(CommonRenderShareName::GBuffer(0), std::make_shared<SharedParam<Ptr<Texture>>>(gbuffer0));
		sharedEnviroment->SetParam(CommonRenderShareName::GBuffer(1), std::make_shared<SharedParam<Ptr<Texture>>>(gbuffer1));
		sharedEnviroment->SetParam("ReflectionMapID", std::make_shared<SharedParam<Ptr<Texture>>>(reflectionMapID));
		sharedEnviroment->SetParam(CommonRenderShareName::RawDepth(), std::make_shared<SharedParam<Ptr<Texture>>>(rawDepthTex));
		sharedEnviroment->SetParam(CommonRenderShareName::LinearDepth(), std::make_shared<SharedParam<Ptr<Texture>>>(linearDepth));
		sharedEnviroment->SetParam(CommonRenderShareName::Velocity(), std::make_shared<SharedParam<Ptr<Texture>>>(velocity));
	}


	DeferredRenderingLighting::DeferredRenderingLighting()
	{
		_deferredFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DeferredRenderingLighting.xml");
	}

	void DeferredRenderingLighting::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();
		auto factory = re->GetRenderFactory();

		TextureDesc desc;
		desc.width = static_cast<int32_t>(sharedEnviroment->GetView()->GetViewport().width);
		desc.height = static_cast<int32_t>(sharedEnviroment->GetView()->GetViewport().height);
		desc.depth = 1;
		desc.arraySize = 1;
		desc.cpuAccess = 0;
		desc.mipLevels = 1;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.type = TEXTURE_2D;

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		desc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		auto lighting0 = factory->GetTexturePooled(desc);
		auto lighting1 = factory->GetTexturePooled(desc);

		//Clear
		rc->ClearRenderTargets({ lighting0->CreateTextureView(), lighting1->CreateTextureView() }, 0.0f);

		//Render
		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto gbuffer1 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto linearDepth = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		//Set Effects Params
		_deferredFX->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_deferredFX->VariableByName("gbuffer1")->AsShaderResource()->SetValue(gbuffer1->CreateTextureView());
		_deferredFX->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepth->CreateTextureView());

		auto & cameraPos = sharedEnviroment->GetView()->GetCamera()->Pos();
		_deferredFX->VariableByName("viewPos")->AsScalar()->SetValue(&cameraPos, sizeof(cameraPos));
		auto & proj = sharedEnviroment->GetView()->GetCamera()->ProjMatrix();
		_deferredFX->VariableByName("proj")->AsScalar()->SetValue(&proj);
		auto &view = sharedEnviroment->GetView()->GetCamera()->ViewMatrix();
		auto viewXM = XMLoadFloat4x4(&view);
		auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
		XMFLOAT4X4 invView;
		XMStoreFloat4x4(&invView, invViewXM);
		_deferredFX->VariableByName("invView")->AsScalar()->SetValue(&invView);

		float2 camNearFar(sharedEnviroment->GetView()->GetCamera()->Near(), sharedEnviroment->GetView()->GetCamera()->Far());
		_deferredFX->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar, sizeof(camNearFar));

		_deferredFX->SetExtraMacros({});

		//Do Render
		for (auto & light : sharedEnviroment->GetView()->GetRenderLights())
		{
			light->BindMacros(_deferredFX, false, sharedEnviroment->GetView()->GetCamera());
			_deferredFX->UpdateData();
			light->BindParams(_deferredFX, false, sharedEnviroment->GetView()->GetCamera());

			rc->SetRenderTargets({ lighting0->CreateTextureView(), lighting1->CreateTextureView() }, 0);

			RenderQuad(_deferredFX->TechniqueByName("Lighting"), 0, 0, desc.width, desc.height);
		}

		//IBL
		std::set<Ptr<ReflectionMap>> reflectionMaps;
		for (auto & obj : sharedEnviroment->GetView()->GetRenderObjects())
		{
			if (obj->GetMaterial()->IsTranslucent())
				continue;
			if (obj->GetReflectionMap())
				reflectionMaps.insert(obj->GetReflectionMap());
		}

		auto reflectionMapIDTex = sharedEnviroment->ParamByName("ReflectionMapID")->As<SharedParam<Ptr<Texture>>>()->GetValue();
		_deferredFX->VariableByName("reflectionMapIDTex")->AsShaderResource()->SetValue(reflectionMapIDTex->CreateTextureView());
		rc->SetRenderTargets({ lighting1->CreateTextureView() }, 0);
		for (auto & map : reflectionMaps)
		{
			auto id = map->GetID();
			_deferredFX->VariableByName("reflectionMapID")->AsScalar()->SetValue(&id, sizeof(id));
			map->BindEffectParams(_deferredFX);
			_deferredFX->TechniqueByName("LightingIBL")->PassByIndex(0)->Bind();
			rc->DrawIndexed();
			_deferredFX->TechniqueByName("LightingIBL")->PassByIndex(0)->UnBind();
		}

		//Set Share
		sharedEnviroment->SetParam(CommonRenderShareName::Lighting(0), std::make_shared<SharedParam<Ptr<Texture>>>(lighting0));
		sharedEnviroment->SetParam(CommonRenderShareName::Lighting(1), std::make_shared<SharedParam<Ptr<Texture>>>(lighting1));
	}


	DeferredRenderingShading::DeferredRenderingShading()
	{
		_deferredFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DeferredRenderingShading.xml");
	}

	void DeferredRenderingShading::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		auto gbuffer0 = sharedEnviroment->ParamByName(CommonRenderShareName::GBuffer(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto lightingTex0 = sharedEnviroment->ParamByName(CommonRenderShareName::Lighting(0))->As<SharedParam<Ptr<Texture>>>()->GetValue();
		auto lightingTex1 = sharedEnviroment->ParamByName(CommonRenderShareName::Lighting(1))->As<SharedParam<Ptr<Texture>>>()->GetValue();

		_deferredFX->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
		_deferredFX->VariableByName("lightingTex0")->AsShaderResource()->SetValue(lightingTex0->CreateTextureView());
		_deferredFX->VariableByName("lightingTex1")->AsShaderResource()->SetValue(lightingTex1->CreateTextureView());

		rc->SetRenderTargets({ sharedEnviroment->GetView()->GetRenderResult()->CreateTextureView() }, 0);

		RenderQuad(_deferredFX->TechniqueByName("Shading"), 0, 0, 
			sharedEnviroment->GetView()->GetRenderResult()->Desc().width,
			sharedEnviroment->GetView()->GetRenderResult()->Desc().height);
	}
}