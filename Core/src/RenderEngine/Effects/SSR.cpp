#include "ToyGE\RenderEngine\Effects\SSR.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	SSR::SSR()
		: _ssrMaxRoughness(0.9f),
		_ssrIntensity(1.0f)
	{
	}

	void SSR::Render(const Ptr<RenderView> & view)
	{
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");

		TextureDesc ssrDesc = sceneTex->GetDesc();
		auto ssrResultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, ssrDesc });
		auto ssrResult = ssrResultTexRef->Get()->Cast<Texture>();

		auto tmpDepthTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneClipDepth->GetDesc() });
		auto tmpDepthTex = tmpDepthTexRef->Get()->Cast<Texture>();
		sceneClipDepth->CopyTo(tmpDepthTex, 0, 0, 0, 0, 0, 0, 0);

		auto ps = Shader::FindOrCreate<ScreenSpaceReflectionPS>();

		view->BindShaderParams(ps);
		ps->SetScalar("frameCount", Global::GetInfo()->frameCount);
		ps->SetScalar("ssrMaxRoughness", _ssrMaxRoughness);
		ps->SetScalar("ssrIntensity", _ssrIntensity);
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("depthTex", tmpDepthTex->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		ps->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView());
		ps->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView());

		ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

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
		{ ssrResult->GetRenderTargetView(0, 0, 1) }, 
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		auto ssrSharedRef = SSRNeighborShare(view, ssrResult, gbuffer1, tmpDepthTex, sceneClipDepth);
		auto ssrShared = ssrSharedRef->Get()->Cast<Texture>();

		// Temporal AA for SSR
		auto ssrAAedResultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
		auto ssrAAedResultTex = ssrAAedResultTexRef->Get()->Cast<Texture>();
		{
			auto adaptedExposureScale = view->GetViewRenderContext()->GetSharedTexture("AdaptedExposureScale");

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

			std::map<String, String> macros;
			macros["TAA_DYNAMIC"] = "0";
			macros["TAA_HISTORY_BICUBIC"] = "0";

			auto ps = Shader::FindOrCreate<TemporalAAPS>(macros);

			view->BindShaderParams(ps);

			ps->SetScalar("texSize", ssrResult->GetTexSize());
			ps->SetScalar("neighborFilterWeights", filterWeights, (int)sizeof(float) * 5);
			ps->SetScalar("frameCount", (uint32_t)Global::GetInfo()->frameCount);
			ps->SetScalar("lerpFactor", 0.125f);
			//ps->SetSRV("linearDepth", sceneLinearClipDepth->GetShaderResourceView());
			ps->SetSRV("sceneDepth", sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
			ps->SetSRV("sceneTex", ssrShared->GetShaderResourceView());
			//ps->SetSRV("velocityTex", velocityTex->GetShaderResourceView());
			ps->SetSRV("adaptedExposureScale", adaptedExposureScale->GetShaderResourceView());
			if (_preSSRResultRef)
				ps->SetSRV("historyTex", _preSSRResultRef->Get()->Cast<Texture>()->GetShaderResourceView());
			else
				ps->SetSRV("historyTex", nullptr);
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

			ps->Flush();

			DrawQuad(
			{ ssrAAedResultTex->GetRenderTargetView(0, 0, 1) },
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
				sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			_preSSRResultRef = ssrResultTexRef;
		}

		rc->SetBlendState(
			BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());


		rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		Transform(
			ssrAAedResultTex->GetShaderResourceView(),
			sceneTex->GetRenderTargetView(0, 0, 1), 
			{ COLOR_WRITE_R, COLOR_WRITE_G ,COLOR_WRITE_B ,COLOR_WRITE_A }, 0.0f, 0.0f, nullptr,
			sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		rc->SetBlendState(nullptr);
		rc->SetDepthStencilState(nullptr);
	}

	PooledTextureRef SSR::BuildHZB(const Ptr<Texture> & depthTex)
	{
		auto texDesc = depthTex->GetDesc();
		texDesc.format = RENDER_FORMAT_R16_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.mipLevels = 0;

		auto hzb0Ref = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto hzb1Ref = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		auto hzb0 = hzb0Ref->Get()->Cast<Texture>();
		auto hzb1 = hzb1Ref->Get()->Cast<Texture>();

		Transform(
			depthTex->GetShaderResourceView(0, 1, 0, 1, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS), 
			hzb0Ref->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto ps = Shader::FindOrCreate<HZBBuildPS>();

		for (int32_t mipLevel = 1; mipLevel < hzb0->GetDesc().mipLevels; ++mipLevel)
		{
			auto & mipSize = hzb0->GetMipSize(mipLevel - 1);
			auto w = mipSize.x();
			auto h = mipSize.y();
			float4 screenSize = float4((float)w, (float)h, 1.0f / (float)w, 1.0f / (float)h);

			ps->SetScalar("screenSize", screenSize);
			ps->SetScalar("mipLevel", (float)(mipLevel - 1));
			ps->SetSRV("depthTex", hzb0->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

			ps->Flush();

			auto & targetMipSize = hzb0->GetMipSize(mipLevel);

			DrawQuad({ hzb1->GetRenderTargetView(mipLevel, 0, 1) });

			hzb1->CopyTo(hzb0, mipLevel, 0, 0, 0, 0, mipLevel, 0);
		}

		return hzb0Ref;
	}

	PooledTextureRef SSR::SSRNeighborShare(
		const Ptr<RenderView> & view,
		const Ptr<Texture> & ssrResult,
		const Ptr<Texture> & gbuffer1,
		const Ptr<Texture> & depthTex,
		const Ptr<Texture> & ssrStencilMask)
	{
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, ssrResult->GetDesc() });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<SSRNeighborSharePS>();
		view->BindShaderParams(ps);
		ps->SetScalar("texSize", ssrResult->GetTexSize());
		ps->SetSRV("ssrTex", ssrResult->GetShaderResourceView());
		ps->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView());
		ps->SetSRV("depthTex", depthTex->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->Flush();

		DrawQuad({ resultTex->GetRenderTargetView(0, 0, 1) },
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			ssrStencilMask->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		return resultTexRef;
	}
}