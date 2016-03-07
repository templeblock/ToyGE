#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\SceneRenderer.h"
#include "ToyGE\RenderEngine\DeferredSceneRenderer.h"
#include "ToyGE\RenderEngine\OctreeCuller.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\Font\FontRenderer.h"
#include "ToyGE\RenderEngine\Effects\FrameInfoRender.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, GammaCorrectionPS, SHADER_PS, "GammaCorrection", "GammaCorrectionPS", SM_4);

	ShaderModel RenderEngine::_shaderModel;

	RenderEngine::~RenderEngine()
	{
		::TwTerminate();
	}

	void RenderEngine::Init(const RenderEngineInitParams & initParams)
	{
		if(!_sceneRender)
			_sceneRender = std::make_shared<DeferredSceneRenderer>();

		if(!_sceneRenderObjsCuller)
			SetSceneRenderObjsCuller(std::make_shared<DefaultRenderObjectCuller>());

		if (!_sceneRenderLightsCuller)
			SetSceneRenderLightsCuller(std::make_shared<DefaultRenderLightCuller>());
	}

	void RenderEngine::Render()
	{
		std::set<Ptr<LightComponent>> lightsWithShadow;
		std::set<Ptr<LightComponent>> lightsWithShadowNoRelevantView;
		std::set<Ptr<LightComponent>> lightsWithRSM;
		std::set<Ptr<LightComponent>> lightsWithRSMNoRelevantView;

		for (int32_t i = 0; i < Global::GetScene()->NumViews(); ++i)
		{
			auto view = Global::GetScene()->GetView(i);
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

				if (light->IsCastLPV() && light->GetShadowTechnique())
				{
					if (light->GetShadowTechnique()->IsRelevantWithView())
					{
						light->GetShadowTechnique()->SetLight(light);
						light->GetShadowTechnique()->PrepareRSM(view);
					}
					else
					{
						lightsWithRSMNoRelevantView.insert(light);
					}

					lightsWithRSM.insert(light);
				}
			}
		}

		for (auto & light : lightsWithShadowNoRelevantView)
		{
			light->GetShadowTechnique()->SetLight(light);
			light->GetShadowTechnique()->PrepareShadow(nullptr);
		}
		for (auto & light : lightsWithRSMNoRelevantView)
		{
			light->GetShadowTechnique()->SetLight(light);
			light->GetShadowTechnique()->PrepareRSM(nullptr);
		}

		for (int32_t i = 0; i < Global::GetScene()->NumViews(); ++i)
		{
			auto view = Global::GetScene()->GetView(i);
			GetSceneRenderer()->Render(view);
		}

		for (int32_t i = 0; i < Global::GetScene()->NumViews(); ++i)
		{
			auto view = Global::GetScene()->GetView(i);
			view->PostRender();
		}

		// Render info
		FrameInfoRender::Render(_frameBuffer->GetRenderTargetView(0, 0, 1));

		// GammaCorrection
		/*auto gammaCorrectionPS = Shader::FindOrCreate<GammaCorrectionPS>();
		gammaCorrectionPS->SetScalar("gamma", _gamma);
		gammaCorrectionPS->SetSRV("inTex", _frameBuffer->GetShaderResourceView());
		gammaCorrectionPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		gammaCorrectionPS->Flush();
		DrawQuad({ _backBuffer->GetRenderTargetView(0, 0, 1) });*/

		Transform(
			_frameBuffer->GetShaderResourceView(), 
			_backBuffer->GetRenderTargetView(0, 0, 1), 
			{ COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A }, 
			0.0f, 0.0f, 
			SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		SwapChain();

		for (auto & light : lightsWithShadow)
		{
			light->GetShadowTechnique()->ClearShadow();
		}
		for (auto & light : lightsWithRSM)
		{
			light->GetShadowTechnique()->ClearRSM();
		}
	}

	void RenderEngine::SetFullScreen(bool bFullScreen)
	{
		_bFullScreen = bFullScreen;
	}
}

