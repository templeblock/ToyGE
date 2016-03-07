#include "ToyGE\RenderEngine\ReflectionMapCapture.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderEngineInclude.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"

namespace ToyGE
{
	Ptr<class ReflectionMap> ReflectionMapCapture::CaptureScene(const Ptr<class Scene> & scene)
	{
		auto reflectionMap = std::make_shared<ReflectionMap>();

		static std::vector<float3> viewDir =
		{
			float3(1.0f, 0.0f, 0.0f),
			float3(-1.0f, 0.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, -1.0f, 0.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 0.0f, -1.0f)
		};
		static std::vector<float3> upDir =
		{
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 0.0f, -1.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f)
		};

		// Init target texture
		TextureDesc texDesc;
		texDesc.width = texDesc.height = 128;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.bCube = true;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		auto targetTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto targetTex = targetTexRef->Get()->Cast<Texture>();

		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = (float)texDesc.width;
		vp.height = (float)texDesc.height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		

		// Init views
		std::array<Ptr<RenderView>, 6> views;

		int viewIndex = 0;
		for (auto & view : views)
		{
			view = std::make_shared<RenderView>();
			view->sceneRenderingConfig.bGenVelocityMap = false;
			view->sceneRenderingConfig.bLPV = false;
			view->sceneRenderingConfig.bOIT = true;
			//view->sceneRenderingConfig.bRenderingAtmosphere = false;
			view->sceneRenderingConfig.bSSR = false;
			view->sceneRenderingConfig.bTAA = false;
			view->sceneRenderingConfig.bRenderEnvReflection = false;
			view->sceneRenderingConfig.bRenderAmbient = false;

			view->SetViewport(vp);

			auto camera = std::make_shared<PerspectiveCamera>(PI_DIV2, 1.0f, 0.1f, _radius);
			camera->LookTo(_pos, viewDir[viewIndex], upDir[viewIndex]);
			view->SetCamera(camera);
			view->SetScene(scene);
			view->SetRenderTarget(targetTex->GetRenderTargetView(0, viewIndex, 1));

			++viewIndex;
		}

		// Render
		std::set<Ptr<LightComponent>> lightsWithShadow;
		std::set<Ptr<LightComponent>> lightsWithShadowNoRelevantView;
		for (auto & view : views)
		{
			view->PreRender();

			for (auto & light : view->GetViewRenderContext()->lights)
			{
				if (light->IsCastShadow() && light->GetShadowTechnique())
				{
					if (light->GetShadowTechnique()->IsRelevantWithView())
					{
						light->GetShadowTechnique()->SetLight(light);
						light->GetShadowTechnique()->PrepareShadow(view);
					}
					else
					{
						lightsWithShadowNoRelevantView.insert(light);
					}

					lightsWithShadow.insert(light);
				}
			}
		}

		for (auto & light : lightsWithShadowNoRelevantView)
		{
			light->GetShadowTechnique()->SetLight(light);
			light->GetShadowTechnique()->PrepareShadow(nullptr);
		}

		for (auto & view : views)
		{
			Global::GetRenderEngine()->GetSceneRenderer()->Render(view);
			view->PostRender();
		}

		for (auto & light : lightsWithShadow)
		{
			light->GetShadowTechnique()->ClearShadow();
		}

		reflectionMap->SetEnvironmentMap(targetTex);
		reflectionMap->InitPreComputedData();

		return reflectionMap;
	}
}