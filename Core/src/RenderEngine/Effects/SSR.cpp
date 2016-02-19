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

		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
		sceneTex->CopyTo(resultTexRef->Get()->Cast<Texture>(), 0, 0, 0, 0, 0, 0, 0);

		auto ps = Shader::FindOrCreate<ScreenSpaceReflectionPS>();

		view->BindShaderParams(ps);
		ps->SetScalar("frameCount", Global::GetInfo()->frameCount);
		ps->SetScalar("ssrMaxRoughness", _ssrMaxRoughness);
		ps->SetScalar("ssrIntensity", _ssrIntensity);
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("depthTex", sceneClipDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		ps->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView());
		ps->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView());

		ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(
			BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD>::Get());

		DrawQuad({ resultTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		view->GetViewRenderContext()->SetSharedResource("RenderResult", resultTexRef);

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
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
}