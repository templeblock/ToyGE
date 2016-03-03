#include "ToyGE\RenderEngine\Effects\ToneMapping.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Blur.h"

namespace ToyGE
{
	ToneMapping::ToneMapping()
		: _brightPassThreshold(1.5f)
	{
	}

	void ToneMapping::Render(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneDownSampleTex = view->GetViewRenderContext()->GetSharedTexture("HalfScene");
		auto adaptedExposureScale = view->GetViewRenderContext()->GetSharedTexture("AdaptedExposureScale");

		// Bright pass
		auto brightPassTexRef = BrightPass(sceneDownSampleTex, adaptedExposureScale);
		auto brightPassTex = brightPassTexRef->Get()->Cast<Texture>();

		static const int32_t numBlurTex = 6;

		// Bloom down sample
		std::vector<PooledRenderResourceReference<Texture>> bloomDownSampleTexList;
		bloomDownSampleTexList.push_back(brightPassTexRef);
		for (int32_t i = 1; i < numBlurTex; ++i)
		{
			bloomDownSampleTexList.push_back(BloomDownSample(bloomDownSampleTexList.back()->Get()->Cast<Texture>()));
		}

		// Bloom blur
		float radius[numBlurTex] = 
		{
			16.0f,
			16.0f,
			16.0f,
			16.0f,
			16.0f,
			16.0f
		};
		for (int i = 0; i < numBlurTex; ++i)
		{
			/*Blur::GaussBlur(
				bloomDownSampleTexList[i]->Get()->Cast<Texture>()->GetShaderResourceView(),
				bloomDownSampleTexList[i]->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
				std::min<int32_t>(31, (int32_t)radius[i] * 2 + 1), radius[i]);*/

			BloomBlur(bloomDownSampleTexList[i]->Get()->Cast<Texture>(), radius[i]);
		}

		// Bloom up sample
		for (int i = numBlurTex - 2; i >= 0; --i)
		{
			BloomUpSample(
				1.0f / (float)numBlurTex,
				bloomDownSampleTexList[i + 1]->Get()->Cast<Texture>(), 
				bloomDownSampleTexList[i]->Get()->Cast<Texture>());
		}

		/*auto bloomFinalDesc = bloomDownSampleTexList[0]->Get()->Cast<Texture>()->GetDesc();
		auto bloomFinalRef = TexturePool::Instance().FindFree({ TEXTURE_2D, bloomFinalDesc });
		auto bloomFinal = bloomFinalRef->Get()->Cast<Texture>();
		BloomUpSample(
			1.0f / (float)numBlurTex,
			bloomDownSampleTexList[0]->Get()->Cast<Texture>(),
			bloomFinal);*/

		//// Streak
		//auto streakTexDesc = brightPassTexRef->Get()->Cast<Texture>()->GetDesc();
		//auto streakTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, streakTexDesc });
		//auto streakTexTmpRef = TexturePool::Instance().FindFree({ TEXTURE_2D, streakTexDesc });

		//Blur::BoxBlurX(
		//	brightPassTexRef->Get()->Cast<Texture>()->GetShaderResourceView(), 
		//	streakTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 
		//	5, 3.0f);
		//for (int i = 0; i < 5; ++i)
		//{
		//	Blur::BoxBlurX(
		//		streakTexRef->Get()->Cast<Texture>()->GetShaderResourceView(),
		//		streakTexTmpRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
		//		5, 3.0f);
		//	streakTexRef.swap(streakTexTmpRef);
		//}

		auto lutRef = ComputeToneMappingLUT();

		auto newTargetDesc = sceneTex->GetDesc();
		auto newTargetRef = TexturePool::Instance().FindFree({ TEXTURE_2D, newTargetDesc });

		DoToneMapping(
			sceneTex, 
			bloomDownSampleTexList[0]->Get()->Cast<Texture>(),
			adaptedExposureScale,
			lutRef->Get()->Cast<Texture>(),
			newTargetRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1));

		view->GetViewRenderContext()->SetSharedResource("RenderResult", newTargetRef);
	}

	PooledTextureRef ToneMapping::BrightPass(const Ptr<Texture> & sceneTex, const Ptr<Texture> & adaptedExposureScale)
	{
		auto texDesc = sceneTex->GetDesc();
		texDesc.mipLevels = 1;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
		auto brightPassTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto brightPassTex = brightPassTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<BrightPassPS>();

		ps->SetScalar("brightPassThreshold", _brightPassThreshold);
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		ps->SetSRV("adaptedExposureScale", adaptedExposureScale->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ brightPassTex->GetRenderTargetView(0, 0, 1) });

		return brightPassTexRef;
	}

	PooledRenderResourceReference<Texture> ToneMapping::BloomDownSample(const Ptr<Texture> & inTex)
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

	void ToneMapping::BloomBlur(const Ptr<Texture> & inTex, float blurRadius)
	{
		TextureDesc texDesc = inTex->GetDesc();
		auto tmpTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto tmpTex = tmpTexRef->Get()->Cast<Texture>();

		{
			int32_t tableSize = (int32_t)blurRadius * 2 + 1;
			auto & gaussTable = GetGaussTable(tableSize);
			int32_t sampleIndex = 0;
			float weightSum = 0.0f;
			std::vector<float2> offsets;
			std::vector<float> weights;
			for (; sampleIndex < tableSize - 1; sampleIndex += 2)
			{
				float offsetStart = (float)sampleIndex - blurRadius;

				float weight0 = gaussTable[sampleIndex];
				float weight1 = gaussTable[sampleIndex + 1];
				weights.push_back(weight0 + weight1);
				offsets.push_back(float2(1.0f, 0.0f) * (offsetStart + weight1 / weights.back()));
				weightSum += weights.back();
			}
			weights.push_back(gaussTable.back());
			offsets.push_back(float2(1.0f, 0.0f) * blurRadius);

			for (auto & w : weights)
				w /= weightSum;
			for (auto & offset : offsets)
				offset /= float2((float)inTex->GetDesc().width, (float)inTex->GetDesc().height);

			TextureFilter(inTex->GetShaderResourceView(), tmpTex->GetRenderTargetView(0, 0, 1), offsets, weights, SamplerTemplate<FILTER_MIN_MAG_MIP_LINEAR>::Get());
		}

		{
			int32_t tableSize = (int32_t)blurRadius * 2 + 1;
			auto & gaussTable = GetGaussTable(tableSize);
			int32_t sampleIndex = 0;
			float weightSum = 0.0f;
			std::vector<float2> offsets;
			std::vector<float> weights;
			for (; sampleIndex < tableSize - 1; sampleIndex += 2)
			{
				float offsetStart = (float)sampleIndex - blurRadius;

				float weight0 = gaussTable[sampleIndex];
				float weight1 = gaussTable[sampleIndex + 1];
				weights.push_back(weight0 + weight1);
				offsets.push_back(float2(0.0f, 1.0f) * (offsetStart + weight1 / weights.back()));
				weightSum += weights.back();
			}
			weights.push_back(gaussTable.back());
			offsets.push_back(float2(0.0f, 1.0f) * blurRadius);

			for (auto & w : weights)
				w /= weightSum;
			for (auto & offset : offsets)
				offset /= float2((float)inTex->GetDesc().width, (float)inTex->GetDesc().height);

			TextureFilter(tmpTex->GetShaderResourceView(), inTex->GetRenderTargetView(0, 0, 1), offsets, weights, SamplerTemplate<FILTER_MIN_MAG_MIP_LINEAR>::Get());
		}

	}

	void ToneMapping::BloomUpSample(const float & weight, const Ptr<Texture> & lowResTex, const Ptr<Texture> & highResTex)
	{
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_ONE>::Get());
		Transform(lowResTex->GetShaderResourceView(), highResTex->GetRenderTargetView(0, 0, 1));
		//TextureFilter(lowResTex->GetShaderResourceView(), highResTex->GetRenderTargetView(0, 0, 1), { float2(0.0f, 0.0f) }, { weight });
		Global::GetRenderEngine()->GetRenderContext()->SetBlendState(nullptr);
	}

	PooledTextureRef ToneMapping::ComputeToneMappingLUT()
	{
		TextureDesc lutTexDesc;
		lutTexDesc.width = lutTexDesc.height = lutTexDesc.depth = 32;
		lutTexDesc.arraySize = 1;
		lutTexDesc.bCube = false;
		lutTexDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
		lutTexDesc.cpuAccess = 0;
		lutTexDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		lutTexDesc.mipLevels = 1;
		lutTexDesc.sampleCount = 1;
		lutTexDesc.sampleQuality = 0;
		auto lutTexRef = TexturePool::Instance().FindFree({ TEXTURE_3D, lutTexDesc });
		auto lutTex = lutTexRef->Get()->Cast<Texture>();

		float WhiteTemp = 6500.0f;
		float WhiteTint = 0.0f;

		float3 ColorSaturation = 1.0f;
		float3 ColorContrast = 1.0f;
		float3 ColorGamma = 1.0f;
		float3 ColorGain = 1.0f;
		float3 ColorOffset = 0.0f;

		float FilmSlope = 0.88f;
		float FilmToe = 0.55f;
		float FilmShoulder = 0.26f;
		float FilmBlackClip = 0.0f;
		float FilmWhiteClip = 0.04f;

		auto ps = Shader::FindOrCreate<ToneMappingLUTPS>();
		ps->SetScalar("WhiteTemp", WhiteTemp);
		ps->SetScalar("WhiteTint", WhiteTint);

		ps->SetScalar("ColorSaturation", ColorSaturation);
		ps->SetScalar("ColorContrast", ColorContrast);
		ps->SetScalar("ColorGamma", ColorGamma);
		ps->SetScalar("ColorGain", ColorGain);
		ps->SetScalar("ColorOffset", ColorOffset);

		ps->SetScalar("FilmSlope", FilmSlope);
		ps->SetScalar("FilmToe", FilmToe);
		ps->SetScalar("FilmShoulder", FilmShoulder);
		ps->SetScalar("FilmBlackClip", FilmBlackClip);
		ps->SetScalar("FilmWhiteClip", FilmWhiteClip);

		ps->Flush();

		RenderToVolumeTexture(lutTex);

		return lutTexRef;
	}

	void ToneMapping::DoToneMapping(
		const Ptr<Texture> & scene,
		const Ptr<Texture> & bloomTex,
		const Ptr<Texture> & adaptedExposureScale,
		const Ptr<Texture> & lut,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<ToneMappingPS>();
		ps->SetScalar("gamma", Global::GetRenderEngine()->GetGamma());
		ps->SetSRV("sceneTex", scene->GetShaderResourceView());
		ps->SetSRV("bloomTex", bloomTex->GetShaderResourceView());
		ps->SetSRV("adaptedExposureScale", adaptedExposureScale->GetShaderResourceView());
		ps->SetSRV("toneMappingLUT", lut->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
		ps->Flush();

		DrawQuad({ target });
	}
}