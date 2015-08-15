#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\RenderEngine\Effects\DebugInfoRender.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\Kernel\Timer.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\DeferredSceneRenderer.h"

namespace ToyGE
{
	DeferredRenderFramework::DeferredRenderFramework()
	{
	}

	void DeferredRenderFramework::Init()
	{
		_sceneRenderer = std::make_shared<DeferredSceneRenderer>();
		_debugInfoRender = std::make_shared<DebugInfoRender>();
	}

	void DeferredRenderFramework::Render()
	{
		if (!Global::GetScene())
			return;

		auto re = Global::GetRenderEngine();
		auto rc = re->GetRenderContext();

		/*Timer timer;
		timer.Upate();*/

		//Cull
		auto scene = Global::GetScene();
		auto numViews = scene->NumViews();
		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			renderView->InitForRender();
		}
		/*float time = timer.Upate();
		DebugInfo::Instance()->SetValue("CullTime", time);*/

		//Render Scene Base
		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			_sceneRenderer->RenderBase(renderView);
		}

		//Prepare Shadows
		std::set<Ptr<LightComponent>> lights;
		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			for (auto & light : renderView->GetRenderLights())
			{
				if (light->IsCastShadow())
				{
					auto shadowTech = light->GetShadowTechnique();
					if (shadowTech->IsRelevantWithCamera())
						shadowTech->PrepareShadow(light, renderView->GetRenderSharedEnviroment());
					else
						lights.insert(light);
				}
			}
		}
		for (auto & light : lights)
		{
			auto shadowTech = light->GetShadowTechnique();
			shadowTech->PrepareShadow(light, nullptr);
		}


		//Render Scene Shading
		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			_sceneRenderer->RenderShading(renderView);
		}

		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			renderView->RenderPostProcess();
		}

		//Render DebugInfo
		_debugInfoRender->Render(scene->GetView(0)->GetRenderSharedEnviroment());

		//Init Render Target
		auto factory = Global::GetRenderEngine()->GetRenderFactory();
		TextureDesc texDesc;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.depth = 1;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.width = Global::GetRenderEngine()->GetWindow()->Width();
		texDesc.height = Global::GetRenderEngine()->GetWindow()->Height();
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		auto targetTex = factory->GetTexturePooled(texDesc);

		rc->ClearRenderTargets({ targetTex->CreateTextureView() }, 0.0f);

		for (int32_t viewIndex = 0; viewIndex < numViews; ++viewIndex)
		{
			auto renderView = scene->GetView(viewIndex);
			int4 dstRect;
			dstRect.x = static_cast<int32_t>(renderView->GetViewport().topLeftX);
			dstRect.y = static_cast<int32_t>(renderView->GetViewport().topLeftY);
			dstRect.z = static_cast<int32_t>(renderView->GetViewport().width);
			dstRect.w = static_cast<int32_t>(renderView->GetViewport().height);

			Transform(
				renderView->GetRenderResult()->CreateTextureView(),
				targetTex->CreateTextureView(),
				Vector4<ColorWriteMask>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A),
				dstRect);
		}

		Global::GetRenderEngine()->PresentToBackBuffer(targetTex->CreateTextureView());

		targetTex->Release();
	}
}