#include "ToyGE\RenderEngine\Effects\HDR.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	HDR::HDR()
		: _brightPassThreshold(0.5f),
		_brightPassScaleParam(2.0f)
	{
	}

	void HDR::Render(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		if (!_prevAvgAdaptedIlumTexRef || !_avgAdaptedIlumTexRef)
		{
			TextureDesc texDesc;
			texDesc.width = texDesc.height = texDesc.depth = 1;
			texDesc.mipLevels = texDesc.arraySize = 1;
			texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			texDesc.cpuAccess = 0;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;
			texDesc.bCube = false;

			_prevAvgAdaptedIlumTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			_avgAdaptedIlumTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

			rc->ClearRenderTarget(_prevAvgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
			rc->ClearRenderTarget(_avgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
		}

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		// Scene down sample
		auto sceneDownSampleTexRef = SceneDownSample(sceneTex);

		// Compute illum
		ComputeIlluminace(sceneDownSampleTexRef->Get()->Cast<Texture>());

		// Bright pass
		auto brightPassTexRef = BrightPass(sceneDownSampleTexRef->Get()->Cast<Texture>());

		int32_t numBlurTex = 5;

		// Bloom down sample
		std::vector<PooledRenderResourceReference<Texture>> bloomDownSampleTexList;
		bloomDownSampleTexList.push_back(brightPassTexRef);
		for (int32_t i = 1; i < numBlurTex; ++i)
		{
			bloomDownSampleTexList.push_back(BloomDownSample(bloomDownSampleTexList.back()->Get()->Cast<Texture>()));
		}

		// Bloom blur
		for (int i = 0; i < numBlurTex; ++i)
		{
			Blur::BoxBlur(
				bloomDownSampleTexList[i]->Get()->Cast<Texture>()->GetShaderResourceView(),
				bloomDownSampleTexList[i]->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
				3, 1.0f);
		}

		// Bloom up sample
		for (int i = numBlurTex - 2; i >= 0; --i)
		{
			BloomUpSample(
				bloomDownSampleTexList[i + 1]->Get()->Cast<Texture>(), 
				bloomDownSampleTexList[i]->Get()->Cast<Texture>());
		}

		// Streak
		auto streakTexDesc = brightPassTexRef->Get()->Cast<Texture>()->GetDesc();
		auto streakTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, streakTexDesc });
		auto streakTexTmpRef = TexturePool::Instance().FindFree({ TEXTURE_2D, streakTexDesc });

		Blur::GaussBlurX(
			brightPassTexRef->Get()->Cast<Texture>()->GetShaderResourceView(), 
			streakTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 
			5, 2.0f);
		for (int i = 0; i < 3; ++i)
		{
			Blur::GaussBlurX(
				streakTexRef->Get()->Cast<Texture>()->GetShaderResourceView(),
				streakTexTmpRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
				5, 2.0f);
			streakTexRef.swap(streakTexTmpRef);
		}

		auto newTargetDesc = sceneTex->GetDesc();
		auto newTargetRef = TexturePool::Instance().FindFree({ TEXTURE_2D, newTargetDesc });

		Combine(sceneTex, 
			bloomDownSampleTexList[0]->Get()->Cast<Texture>(), 
			streakTexRef->Get()->Cast<Texture>(),
			newTargetRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		view->GetViewRenderContext()->SetSharedResource("RenderResult", newTargetRef);
	}

	PooledRenderResourceReference<Texture> HDR::SceneDownSample(const Ptr<Texture> & sceneTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = sceneTex->GetDesc();
		texDesc.mipLevels = 1;
		texDesc.width = texDesc.width / 4;
		texDesc.height = texDesc.height / 4;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		auto downSampleTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		Transform(sceneTex->GetShaderResourceView(), downSampleTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		return downSampleTexRef;
	}

	void HDR::ComputeIlluminace(const Ptr<Texture> & downSampleTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = downSampleTex->GetDesc();
		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		texDesc.width = texDesc.height = 64;
		auto tex0Ref = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto tex1Ref = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto tex0 = tex0Ref->Get()->Cast<Texture>();
		auto tex1 = tex1Ref->Get()->Cast<Texture>();

		auto sampler = SamplerTemplate<
			FILTER_MIN_MAG_MIP_POINT, 
			TEXTURE_ADDRESS_BORDER, 
			TEXTURE_ADDRESS_BORDER, 
			TEXTURE_ADDRESS_BORDER>::Get();

		auto ps = Shader::FindOrCreate<ComputeIlluminaceInitialPS>();
		ps->SetScalar("texSize", downSampleTex->GetTexSize());
		ps->SetSRV("sceneDownSampleTex", downSampleTex->GetShaderResourceView());
		ps->SetSampler("pointBorderSampler", sampler);
		ps->Flush();
		DrawQuad({ tex0->GetRenderTargetView(0, 0, 1) });

		ps = Shader::FindOrCreate<ComputeIlluminaceReducePS>();
		//Reduce calculate ilumance
		float w = ceil(static_cast<float>(texDesc.width) / 4.0f);
		float h = ceil(static_cast<float>(texDesc.height) / 4.0f);
		while (w >= 1.0f || h >= 1.0f)
		{
			ps->SetScalar("texSize", float4(w * 4.0f, h * 4.0f, 1.0f / w / 4.0f, 1.0f / h / 4.0f));
			ps->SetSRV("illumReduceTex", tex0->GetShaderResourceView());
			ps->SetSampler("pointBorderSampler", sampler);
			ps->Flush();

			DrawQuad({ tex1->GetRenderTargetView(0, 0, 1) }, 0.0f, 0.0f, w, h);

			tex0.swap(tex1);

			if (w == 1.0f && h == 1.0f)
				break;

			w = ceil(w / 4.0f);
			h = ceil(h / 4.0f);
		}

		_prevAvgAdaptedIlumTexRef.swap(_avgAdaptedIlumTexRef);

		ps = Shader::FindOrCreate<ComputeAvgAndAdaptedIlumPS>();
		ps->SetScalar("elapsedTime", Global::GetInfo()->elapsedTime);
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSRV("illumReduceTex", tex0->GetShaderResourceView());
		if(_prevAvgAdaptedIlumTexRef)
			ps->SetSRV("prevAvgAdaptedIlumTex", _prevAvgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetShaderResourceView());
		ps->Flush();

		DrawQuad({ _avgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
	}

	PooledRenderResourceReference<Texture> HDR::BrightPass(const Ptr<Texture> & downSampleTex)
	{
		auto texDesc = downSampleTex->GetDesc();
		texDesc.mipLevels = 1;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		auto brightPassTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		auto ps = Shader::FindOrCreate<BrightPassPS>();

		ps->SetScalar("brightPassThreshold", _brightPassThreshold);
		ps->SetScalar("brightPassScale", _brightPassScaleParam);
		ps->SetSRV("sceneDownSampleTex", downSampleTex->GetShaderResourceView());
		ps->SetSRV("avgAdaptedIlumTex", _avgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ brightPassTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return brightPassTexRef;
	}

	PooledRenderResourceReference<Texture> HDR::BloomDownSample(const Ptr<Texture> & inTex)
	{
		auto texDesc = inTex->GetDesc();
		texDesc.mipLevels = 1;
		texDesc.width = std::max<int32_t>(1, texDesc.width / 2);
		texDesc.height = std::max<int32_t>(1, texDesc.height / 2);
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		float4 inTexSize = inTex->GetTexSize();
		std::vector<float2> offsets;
		offsets.push_back(float2(-1.0f, -1.0f) * float2(inTexSize.z(), inTexSize.w()));
		offsets.push_back(float2( 1.0f, -1.0f) * float2(inTexSize.z(), inTexSize.w()));
		offsets.push_back(float2(-1.0f,  1.0f) * float2(inTexSize.z(), inTexSize.w()));
		offsets.push_back(float2( 1.0f,  1.0f) * float2(inTexSize.z(), inTexSize.w()));

		TextureFilter(
			inTex->GetShaderResourceView(), 
			resultTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 
			offsets, 
			std::vector<float>(4, 0.25f));

		return resultTexRef;
	}

	void HDR::BloomUpSample(const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex)
	{
		Transform(lowResTex->GetShaderResourceView(), highResTex->GetRenderTargetView(0, 0, 1));
	}

	void HDR::Combine(
		const Ptr<Texture> & scene,
		const Ptr<Texture> & blurTex,
		const Ptr<Texture> & streakTex,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<HDRCombinePS>();
		ps->SetSRV("sceneTex", scene->GetShaderResourceView());
		ps->SetSRV("blurTex", blurTex->GetShaderResourceView());
		ps->SetSRV("streakTex", streakTex->GetShaderResourceView());
		ps->SetSRV("avgAdaptedIlumTex", _avgAdaptedIlumTexRef->Get()->Cast<Texture>()->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
		ps->Flush();

		DrawQuad({ target });
	}
}