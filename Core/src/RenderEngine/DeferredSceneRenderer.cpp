#include "ToyGE\RenderEngine\DeferredSceneRenderer.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\TranslucentRendering.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\PostProcessing.h"
#include "ToyGE\RenderEngine\Effects\Effects.h"
#include "ToyGE\RenderEngine\Effects\EnvironmentReflectionRenderer.h"

namespace ToyGE
{
	class DeferredMeshDrawingPolicy : public DrawingPolicy
	{
	public:
		Ptr<RenderView> _view;
		Ptr<Material> _mat;
		Ptr<DepthStencilState> _dss;
		bool bTAA;
		bool bGenVelocity;

		virtual void BindView(const Ptr<class RenderView> & view) override
		{
			_view = view;
		}

		virtual void BindMaterial(const Ptr<class Material> & mat) override
		{
			_mat = mat;
		}

		virtual void Draw(const Ptr<class RenderComponent> & renderComponent) override
		{
			if (_mat->IsTranslucent())
				return;

			std::map<String, String> macros;

			if (bGenVelocity || bTAA)
				macros["GEN_VELOCITY"] = "";

			if (_mat)
				_mat->BindMacros(macros);
			else
				Material().BindMacros(macros);

			auto meshElement = renderComponent->GetMeshElement();
			if (meshElement)
				meshElement->BindMacros(macros);

			auto vs = Shader::FindOrCreate<DeferredRenderingBaseVS>(macros);
			auto ps = Shader::FindOrCreate<DeferredRenderingBasePS>(macros);

			_view->BindShaderParams(vs);
			renderComponent->BindShaderParams(vs);

			_view->BindShaderParams(ps);
			if (_mat)
				_mat->BindShaderParams(ps);
			else
				Material().BindShaderParams(ps);

			ps->SetScalar("gamma", Global::GetRenderEngine()->GetGamma());
			ps->SetSampler("bilinearSampler",
				SamplerTemplate<
				FILTER_MIN_MAG_MIP_LINEAR,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP >::Get());

			vs->Flush();
			ps->Flush();

			/*int32_t maskID = 1;
			if (renderComponent->GetReflectionMap())
			{
				maskID = renderComponent->GetReflectionMap()->maskID;
				ToyGE_ASSERT(maskID > 1);
			}*/
			Global::GetRenderEngine()->GetRenderContext()->SetDepthStencilState(
				DepthStencilStateTemplate<
				true,
				DEPTH_WRITE_ALL,
				COMPARISON_LESS,
				true, 0xff, 0xff,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				STENCIL_OP_REPLACE,
				COMPARISON_ALWAYS>::Get(), 1);

			meshElement->Draw();

			renderComponent->StoreWorldTransformCache();
		}
	};

	void DeferredSceneRenderer::Render(const Ptr<RenderView> & view)
	{
		InitBuffers(view);

		Global::GetRenderEngine()->GetRenderContext()->SetViewport(view->GetViewport());

		// Base
		RenderBase(view);

		// Linearize depth
		{
			auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
			auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
			LinearizeDepth(
				sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
				view,
				sceneLinearClipDepth->GetRenderTargetView(0, 0, 1));
		}

		// Lighting
		RenderLighting(view);

		// Shading
		RenderShading(view);

		// Render Ambient
		if(view->sceneRenderingConfig.bRenderAmbient)
		{
			auto renderResult = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
			auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

			if (view->GetScene())
				view->GetScene()->GetAmbientMap()->Render(view);

			if (view->GetScene()->IsRenderSun())
				_atmosphereRendering->RenderSun(view);
		}

		// Translucent
		{
			auto renderResult = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

			_translucentRendering->bOIT = view->sceneRenderingConfig.bOIT;
			_translucentRendering->Render(view, renderResult->GetRenderTargetView(0, 0, 1));
		}

		// Linearize depth
		{
			auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
			auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
			LinearizeDepth(
				sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS),
				view,
				sceneLinearClipDepth->GetRenderTargetView(0, 0, 1));
		}

		// Volumetric Light
		{
			_volumetricLight->Render(view);
		}

		// LPV
		{
			if (view->sceneRenderingConfig.bLPV)
			{
				_lpv->Render(view);
			}
		}

		// SSR
		{
			if (view->sceneRenderingConfig.bSSR)
			{
				_ssrRenderer->SetSSRMaxRoughness(view->sceneRenderingConfig.ssrMaxRoughness);
				_ssrRenderer->SetSSRIntensity(view->sceneRenderingConfig.ssrIntensity);
				_ssrRenderer->Render(view);
			}
		}

		// Render Env Reflection
		if(view->sceneRenderingConfig.bRenderEnvReflection)
		{
			_envReflectionRenderer->Render(view);
		}
		else
		{
			auto ssrResult = view->GetViewRenderContext()->GetSharedTexture("SSR");
			if (ssrResult)
			{
				auto sceneColor = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
				auto sceneDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

				auto rc = Global::GetRenderEngine()->GetRenderContext();

				rc->SetBlendState(
					BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());
				rc->SetDepthStencil(sceneDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

				Transform(
					ssrResult->GetShaderResourceView(),
					sceneColor->GetRenderTargetView(0, 0, 1),
					{ COLOR_WRITE_R, COLOR_WRITE_G ,COLOR_WRITE_B ,COLOR_WRITE_A }, 0.0f, 0.0f, nullptr,
					sceneDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

				rc->SetBlendState(nullptr);
				rc->SetDepthStencilState(nullptr);

			}
		}

		// Half scene
		{
			auto renderResult = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

			auto texDesc = renderResult->GetDesc();
			texDesc.width = texDesc.width / 2;
			texDesc.height = texDesc.height / 2;
			auto halfSceneRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

			Transform(renderResult->GetShaderResourceView(), halfSceneRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));
			view->GetViewRenderContext()->SetSharedResource("HalfScene", halfSceneRef);
		}

		// Eye adaption
		{
			auto renderResult = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

			_eyeAdaption->Render(view);
		}

		// PostProcessing PreTAASetup
		{
			if (view->GetPostProcessing())
				view->GetPostProcessing()->PreTAASetup(view);
		}

		// TAA
		{

			if (view->sceneRenderingConfig.bTAA)
			{
				auto adaptedExposureScale = view->GetViewRenderContext()->GetSharedTexture("AdaptedExposureScale");
				auto renderResult = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
				auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
				auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

				auto newResultRef = TexturePool::Instance().FindFree({ TEXTURE_2D, renderResult->GetDesc() });
				auto newResult = newResultRef->Get()->Cast<Texture>();

				auto velocityTex = view->GetViewRenderContext()->GetSharedTexture("Velocity");

				float2 offsets[5] =
				{
					float2(0.0f, 0.0f),
					float2(-1.0f, 0.0f),
					float2(1.0f, 0.0f),
					float2(0.0f, -1.0f),
					float2(0.0f, 1.0f),
				};
				float filterWeights[5];
				float weightsSum = 0.0f;
				for (int i = 0; i < 5; ++i)
				{
					float2 offset = offsets[i] - float2(0.5f, -0.5f) * view->temporalAAJitter;

					//filterWeights[i] = CatmullRom(offset.x()) * CatmullRom(offset.y());
					offset.x() *= 1.0f + 0.0f * 0.5f;
					offset.y() *= 1.0f + 0.0f * 0.5f;
					filterWeights[i] = exp(-2.29f * (offset.x() * offset.x() + offset.y() * offset.y()));

					weightsSum += filterWeights[i];
				}
				for (auto & i : filterWeights)
					i /= weightsSum;

				auto ps = Shader::FindOrCreate<TemporalAAPS>();

				view->BindShaderParams(ps);

				ps->SetScalar("texSize", renderResult->GetTexSize());
				ps->SetScalar("neighborFilterWeights", filterWeights, (int)sizeof(float) * 5);
				ps->SetScalar("frameCount", (uint32_t)Global::GetInfo()->frameCount);
				ps->SetScalar("lerpFactor", 0.04f);
				//ps->SetSRV("linearDepth", sceneLinearClipDepth->GetShaderResourceView());
				ps->SetSRV("sceneDepth", sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
				ps->SetSRV("sceneTex", renderResult->GetShaderResourceView());
				ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
				ps->SetSRV("adaptedExposureScale", adaptedExposureScale->GetShaderResourceView());
				if (view->preFrameResult)
					ps->SetSRV("historyTex", view->preFrameResult->Get()->Cast<Texture>()->GetShaderResourceView());
				else
					ps->SetSRV("historyTex", nullptr);
				ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
				ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

				ps->Flush();

				DrawQuad({ newResult->GetRenderTargetView(0, 0, 1) });

				view->preFrameResult = newResultRef;
				view->GetViewRenderContext()->SetSharedResource("RenderResult", newResultRef);

			}
			else
			{
				view->preFrameResult = nullptr;
			}
		}

		
	}

	void DeferredSceneRenderer::InitBuffers(const Ptr<RenderView> & view)
	{
		TextureDesc desc;
		desc.width = static_cast<int32_t>(view->GetViewport().width);
		desc.height = static_cast<int32_t>(view->GetViewport().height);
		desc.depth = 1;
		desc.arraySize = 1;
		desc.cpuAccess = 0;
		desc.mipLevels = 1;
		desc.sampleCount = 1;
		desc.sampleQuality = 0;
		desc.bCube = false;

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
		desc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		auto gbuffer0 = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });
		auto gbuffer1 = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

		desc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		auto gbuffer2 = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

		desc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL | TEXTURE_BIND_SHADER_RESOURCE;
		desc.format = RENDER_FORMAT_R24G8_TYPELESS;
		auto sceneClipDepth = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
		desc.format = RENDER_FORMAT_R32_FLOAT;
		auto sceneLinearClipDepth = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

		desc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS;
		desc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		auto lighting0 = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });
		auto lighting1 = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });
		desc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		auto shading = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

		view->GetViewRenderContext()->SetSharedResource("GBuffer0", gbuffer0);
		view->GetViewRenderContext()->SetSharedResource("GBuffer1", gbuffer1);
		view->GetViewRenderContext()->SetSharedResource("GBuffer2", gbuffer2);
		view->GetViewRenderContext()->SetSharedResource("SceneClipDepth", sceneClipDepth);
		view->GetViewRenderContext()->SetSharedResource("SceneLinearClipDepth", sceneLinearClipDepth);
		view->GetViewRenderContext()->SetSharedResource("Lighting0", lighting0);
		view->GetViewRenderContext()->SetSharedResource("Lighting1", lighting1);
		view->GetViewRenderContext()->SetSharedResource("Shading", shading);

		if (view->sceneRenderingConfig.bGenVelocityMap || view->sceneRenderingConfig.bTAA)
		{
			desc.format = RENDER_FORMAT_R16G16_FLOAT;
			auto velocity = TexturePool::Instance().FindFree({ TEXTURE_2D, desc });

			view->GetViewRenderContext()->SetSharedResource("Velocity", velocity);
		}
	}

	void DeferredSceneRenderer::RenderBase(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");
		auto gbuffer2 = view->GetViewRenderContext()->GetSharedTexture("GBuffer2");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		//auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto velocity = view->GetViewRenderContext()->GetSharedTexture("Velocity");

		rc->SetViewport(view->GetViewport());

		if (view->sceneRenderingConfig.bGenVelocityMap || view->sceneRenderingConfig.bTAA)
		{
			rc->SetRenderTargets(
			{
				gbuffer0->GetRenderTargetView(0, 0, 1),
				gbuffer1->GetRenderTargetView(0, 0, 1),
				gbuffer2->GetRenderTargetView(0, 0, 1),
				//sceneLinearClipDepth->GetRenderTargetView(0, 0, 1),
				velocity->GetRenderTargetView(0, 0, 1)
			});
		}
		else
		{
			rc->SetRenderTargets(
			{
				gbuffer0->GetRenderTargetView(0, 0, 1),
				gbuffer1->GetRenderTargetView(0, 0, 1),
				gbuffer2->GetRenderTargetView(0, 0, 1),
				//sceneLinearClipDepth->GetRenderTargetView(0, 0, 1)
			});
		}
		rc->ClearRenderTarget(0.0f);

		rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
		rc->ClearDepthStencil(1.0f, 0);

		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			true,
			DEPTH_WRITE_ALL,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_REPLACE,
			COMPARISON_ALWAYS>::Get(), 1);

		rc->SetRasterizerState(nullptr);
		rc->SetBlendState(nullptr);

		auto drawingPolicy = std::make_shared<DeferredMeshDrawingPolicy>();
		drawingPolicy->bTAA = view->sceneRenderingConfig.bTAA;
		drawingPolicy->bGenVelocity = view->sceneRenderingConfig.bGenVelocityMap;
		view->GetViewRenderContext()->primitiveDrawList->Draw(drawingPolicy, view);

		rc->SetDepthStencilState(nullptr);
	}

	void DeferredSceneRenderer::RenderLighting(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");
		auto gbuffer2 = view->GetViewRenderContext()->GetSharedTexture("GBuffer2");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto lighting0 = view->GetViewRenderContext()->GetSharedTexture("Lighting0");
		auto lighting1 = view->GetViewRenderContext()->GetSharedTexture("Lighting1");

		rc->ClearRenderTarget(
		{ 
			lighting0->GetRenderTargetView(0, 0, 1), 
			lighting1->GetRenderTargetView(0, 0, 1) 
		}, 0.0f);

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			COMPARISON_NOT_EQUAL>::Get(), 0);

		for (auto & light : view->GetViewRenderContext()->lights)
		{
			std::map<String, String> macros;
			light->BindMacros(true, view, macros);

			auto lightingPS = Shader::FindOrCreate<DeferredRenderingLightingPS>(macros);
			
			light->BindShaderParams(lightingPS, true, view);
			view->BindShaderParams(lightingPS);

			lightingPS->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView(0, 1, 0, 1));
			lightingPS->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView(0, 1, 0, 1));
			lightingPS->SetSRV("gbuffer2", gbuffer2->GetShaderResourceView(0, 1, 0, 1));
			lightingPS->SetSRV("sceneLinearClipDepth", sceneLinearClipDepth->GetShaderResourceView(0, 1, 0, 1, false, RENDER_FORMAT_R32_FLOAT));
			lightingPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			lightingPS->Flush();

			DrawQuad(
			{
				lighting0->GetRenderTargetView(0, 0, 1),
				lighting1->GetRenderTargetView(0, 0, 1)
			},
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
				sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
		}

		// IBL
		/*std::set<Ptr<ReflectionMap>> reflectionMaps;
		for (auto & drawBatch : view->GetViewRenderContext()->primitiveDrawList->drawBatches)
		{
			for (auto & renderComponent : drawBatch.second)
			{
				auto reflectionMap = renderComponent->GetReflectionMap();
				if (reflectionMap)
					reflectionMaps.insert(reflectionMap);
			}
		}
		for (auto & reflectionMap : reflectionMaps)
		{
			int32_t maskID = reflectionMap->maskID;

			auto iblPS = Shader::FindOrCreate<IBLPS>();
			reflectionMap->BindShaderParams(iblPS);
			view->BindShaderParams(iblPS);

			iblPS->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView(0, 1, 0, 1));
			iblPS->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView(0, 1, 0, 1));
			iblPS->SetSRV("gbuffer2", gbuffer2->GetShaderResourceView(0, 1, 0, 1));
			iblPS->SetSRV("sceneLinearClipDepth", sceneLinearClipDepth->GetShaderResourceView(0, 1, 0, 1, false, RENDER_FORMAT_R32_FLOAT));
			iblPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			iblPS->Flush();

			rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

			rc->SetDepthStencilState(
				DepthStencilStateTemplate<
				false,
				DEPTH_WRITE_ZERO,
				COMPARISON_LESS,
				true, 0xff, 0xff,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				COMPARISON_EQUAL>::Get(), (uint32_t)maskID);

			DrawQuad(
			{
				lighting1->GetRenderTargetView(0, 0, 1)
			},
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
				sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
		}*/

		rc->SetBlendState(nullptr);
		rc->SetDepthStencilState(nullptr);
	}

	void DeferredSceneRenderer::RenderShading(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto lighting0 = view->GetViewRenderContext()->GetSharedTexture("Lighting0");
		auto lighting1 = view->GetViewRenderContext()->GetSharedTexture("Lighting1");
		auto shading = view->GetViewRenderContext()->GetSharedTexture("Shading");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");

		rc->ClearRenderTarget(shading->GetRenderTargetView(0, 0, 1), 0.0f);

		auto shadingPS = Shader::FindOrCreate<DeferredRenderingShadingPS>();

		shadingPS->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView(0, 1, 0, 1));
		shadingPS->SetSRV("lighting0", lighting0->GetShaderResourceView(0, 1, 0, 1));
		shadingPS->SetSRV("lighting1", lighting1->GetShaderResourceView(0, 1, 0, 1));
		shadingPS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		shadingPS->SetScalar("ambientColor", Global::GetScene()->GetAmbientColor());

		shadingPS->Flush();

		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			COMPARISON_NOT_EQUAL>::Get(), 0);

		DrawQuad(
		{
			shading->GetRenderTargetView(0, 0, 1)
		},
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		rc->SetDepthStencil(nullptr);

		view->GetViewRenderContext()->SetSharedResource("RenderResult", view->GetViewRenderContext()->sharedResources["Shading"]);
	}
}