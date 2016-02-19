#include "ToyGE\RenderEngine\Effects\SSAO.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	SSAO::SSAO()
		: _aoRadius(0.1f),
		_aoPower(20.0f),
		_aoIntensity(0.5f)
	{
	}

	void SSAO::Render(const Ptr<RenderView> & view)
	{
		auto gbuffer1 = view->GetViewRenderContext()->sharedResources["GBuffer1"].Cast<Texture>();
		auto sceneLinearDepthTex = view->GetViewRenderContext()->sharedResources["SceneLinearClipDepth"].Cast<Texture>();

		int32_t numMips = 2;

		std::vector<PooledTextureRef> normalTexs(numMips);
		normalTexs[0] = gbuffer1;
		std::vector<PooledTextureRef> depthTexs(numMips);
		depthTexs[0] = sceneLinearDepthTex;

		for (int32_t i = 1; i < numMips; ++i)
		{
			DownSampleNormalDepth(
				view,
				normalTexs[i - 1]->Get()->Cast<Texture>(), 
				depthTexs[i - 1]->Get()->Cast<Texture>(), 
				normalTexs[i], 
				depthTexs[i]);
		}

		std::vector<PooledTextureRef> aoTexs(numMips);

		float radius = _aoRadius * static_cast<float>(1 << (numMips - 1));

		aoTexs.back() = RenderAOTex(
			radius, 
			view, 
			normalTexs.back()->Get()->Cast<Texture>(), 
			depthTexs.back()->Get()->Cast<Texture>(), 
			nullptr, 
			nullptr, 
			nullptr, 
			false, 
			numMips == 1);

		for (int32_t i = numMips - 2; i >= 0; --i)
		{
			float radius = _aoRadius * static_cast<float>(1 << i);

			aoTexs[i] = RenderAOTex(
				radius,
				view,
				normalTexs[i]->Get()->Cast<Texture>(),
				depthTexs[i]->Get()->Cast<Texture>(), 
				aoTexs[i + 1]->Get()->Cast<Texture>(),
				normalTexs[i + 1]->Get()->Cast<Texture>(),
				depthTexs[i + 1]->Get()->Cast<Texture>(),
				true, 
				i == 0);
		}

		CrossBilateralBlur(view, aoTexs[0]->Get()->Cast<Texture>(), sceneLinearDepthTex->Get()->Cast<Texture>());

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		CombineAO(aoTexs[0]->Get()->Cast<Texture>(), sceneTex);

		view->GetViewRenderContext()->SetSharedResource("AmbientOcclusion", aoTexs[0]);
	}

	void SSAO::DownSampleNormalDepth(
		const Ptr<RenderView> & view,
		const Ptr<Texture> & inNormalTex,
		const Ptr<Texture> & inDepthTex,
		PooledTextureRef & outNormalTex,
		PooledTextureRef & outDepthTex)
	{
		auto texDesc = inNormalTex->GetDesc();
		texDesc.width /= 2;
		texDesc.height /= 2;

		outNormalTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		texDesc.format = inDepthTex->GetDesc().format;
		outDepthTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		float w = static_cast<float>(texDesc.width);
		float h = static_cast<float>(texDesc.height);
		float4 texSize = float4(w, h, 1.0f / w, 1.0f / h);

		auto ps = Shader::FindOrCreate<DownSampleNormalDepthPS>();

		view->BindShaderParams(ps);

		ps->SetScalar("texSize", texSize);
		ps->SetSRV("normalTex", inNormalTex->GetShaderResourceView());
		ps->SetSRV("depthTex", inDepthTex->GetShaderResourceView());
		ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({
			outNormalTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
			outDepthTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
	}

	PooledTextureRef SSAO::RenderAOTex(
		float aoRadius,
		const Ptr<RenderView> & view,
		const Ptr<Texture> & normalTex,
		const Ptr<Texture> & depthTex,
		const Ptr<Texture> & preMipAOTex,
		const Ptr<Texture> & preMipNormalTex,
		const Ptr<Texture> & preMipDepthTex,
		bool bUpSampling,
		bool bFullRes)
	{
		std::map<String, String> macros;
		if (bUpSampling)
			macros["AO_UPSAMPLING"] = "";
		if (bFullRes)
			macros["AO_FULL_RES"] = "";

		auto ps = Shader::FindOrCreate<RenderAOPS>(macros);

		view->BindShaderParams(ps);

		auto camera = std::static_pointer_cast<PerspectiveCamera>(view->GetCamera());
		float invViewRatio = 1.0f / camera->GetAspectRatio();
		ps->SetScalar("invViewRatio", invViewRatio);

		ps->SetScalar("texSize", normalTex->GetTexSize());

		ps->SetScalar("aoPower", _aoPower);
		ps->SetScalar("aoIntensity", _aoIntensity);
		ps->SetScalar("aoRadius", aoRadius);

		ps->SetSRV("depthTex", depthTex->GetShaderResourceView());
		ps->SetSRV("normalTex", normalTex->GetShaderResourceView());
		if(preMipAOTex)
			ps->SetSRV("aoTex", preMipAOTex->GetShaderResourceView());
		if(preMipNormalTex)
			ps->SetSRV("preMipNormalTex", preMipNormalTex->GetShaderResourceView());
		if(preMipDepthTex)
			ps->SetSRV("preMipDepthTex", preMipDepthTex->GetShaderResourceView());

		//ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		auto texDesc = depthTex->GetDesc();
		texDesc.format = RENDER_FORMAT_R8_UNORM;
		auto aoTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		DrawQuad({ aoTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return aoTexRef;
	}

	void SSAO::CrossBilateralBlur(const Ptr<RenderView> & view, const Ptr<Texture> & aoTex, const Ptr<Texture> & depthTex)
	{
		auto texDesc = aoTex->GetDesc();
		auto tmpTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		float4 texSize = depthTex->GetTexSize();

		{
			auto ps = Shader::FindOrCreate<CrossBilateralBlurXPS>();

			view->BindShaderParams(ps);

			ps->SetScalar("texSize", texSize);

			ps->SetSRV("depthTex", depthTex->GetShaderResourceView());
			ps->SetSRV("blurInTex", aoTex->GetShaderResourceView());
			ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ tmpTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
		}

		{
			auto ps = Shader::FindOrCreate<CrossBilateralBlurYPS>();

			view->BindShaderParams(ps);

			ps->SetScalar("texSize", texSize);

			ps->SetSRV("depthTex", depthTex->GetShaderResourceView());
			ps->SetSRV("blurInTex", tmpTexRef->Get()->Cast<Texture>()->GetShaderResourceView());
			ps->SetSampler("pointClampSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ aoTex->GetRenderTargetView(0, 0, 1) });
		}
	}

	void SSAO::CombineAO(const Ptr<Texture> & aoTex, const Ptr<Texture> & target)
	{
		auto ps = Shader::FindOrCreate<CombineAOPS>();

		ps->SetSRV("aoTex", aoTex->GetShaderResourceView());
		ps->Flush();

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(
			BlendStateTemplate<false, false, true, BLEND_PARAM_ZERO, BLEND_PARAM_SRC_ALPHA>::Get());

		DrawQuad({ target->GetRenderTargetView(0, 0, 1) });

		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}
}
