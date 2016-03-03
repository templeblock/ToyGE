#include "ToyGE\RenderEngine\Effects\EyeAdaption.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\GlobalInfo.h"

namespace ToyGE
{
	static const float histogramMin = -8.0f;
	static const float histogramMax = 4.0f;

	void EyeAdaption::Render(const Ptr<RenderView> & view)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		if (!_prevAdaptedExposureScaleRef || !_adaptedExposureScaleRef)
		{
			TextureDesc texDesc;
			texDesc.width = texDesc.height = texDesc.depth = 1;
			texDesc.mipLevels = texDesc.arraySize = 1;
			texDesc.format = RENDER_FORMAT_R32_FLOAT;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
			texDesc.cpuAccess = 0;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;
			texDesc.bCube = false;

			_prevAdaptedExposureScaleRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			_adaptedExposureScaleRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

			rc->ClearRenderTarget(_prevAdaptedExposureScaleRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
			rc->ClearRenderTarget(_adaptedExposureScaleRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
		}

		auto halfSceneTex = view->GetViewRenderContext()->GetSharedTexture("HalfScene");

		auto histogramRef = BuildHistogram(halfSceneTex);
		ComputeEyeAdaption(histogramRef->Get()->Cast<Texture>());

		view->GetViewRenderContext()->SetSharedResource("AdaptedExposureScale", _adaptedExposureScaleRef);
	}

	PooledTextureRef EyeAdaption::BuildHistogram(const Ptr<Texture> & scene)
	{
		static const int32_t histogramSize = 64;
		static const int2 groupSize = 8;
		static const int2 threadLoopSize = 8;

		const int2 texelsInGroup = groupSize * threadLoopSize;
		const int2 numGroups = (int2(scene->GetDesc().width, scene->GetDesc().height) + (texelsInGroup - 1)) / texelsInGroup;

		// Compute histogram
		TextureDesc texDesc;
		texDesc.width = histogramSize;
		texDesc.height = numGroups.x() * numGroups.y();
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.bCube = false;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_UNORDERED_ACCESS | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		auto histogramRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto histogram = histogramRef->Get()->Cast<Texture>();

		auto cs = Shader::FindOrCreate<BuildHistogramCS>();

		float histogramIllumScale = 1.0f / (histogramMax - histogramMin);
		float histogramIllumOffset = -histogramMin * histogramIllumScale;

		cs->SetScalar("histogramIllumScale", histogramIllumScale);
		cs->SetScalar("histogramIllumOffset", histogramIllumOffset);
		cs->SetScalar("numGroups", uint2(numGroups));
		cs->SetScalar("sceneTexSize", uint2(scene->GetDesc().width, scene->GetDesc().height));
		cs->SetSRV("sceneTex", scene->GetShaderResourceView());
		cs->SetUAV("histogramRW", histogram->GetUnorderedAccessView(0, 0, 1));
		cs->Flush();
		Global::GetRenderEngine()->GetRenderContext()->Compute(numGroups.x(), numGroups.y(), 1);
		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);

		// Merge histogram
		texDesc.height = 1;
		auto histogramMergeRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto histogramMerge = histogramMergeRef->Get()->Cast<Texture>();

		auto mergePS = Shader::FindOrCreate<MergeHistogramPS>();
		mergePS->SetScalar("numGroups", (uint32_t)(numGroups.x() * numGroups.y()));
		mergePS->SetScalar("histogramNormalize", 1.0f / (float)(scene->GetDesc().width * scene->GetDesc().height));
		mergePS->SetScalar("histogramTexSize", histogram->GetTexSize());
		mergePS->SetSRV("histogram", histogram->GetShaderResourceView());
		mergePS->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		mergePS->Flush();

		DrawQuad({ histogramMerge->GetRenderTargetView(0, 0, 1) });

		return histogramMergeRef;
	}

	void EyeAdaption::ComputeEyeAdaption(const Ptr<Texture> & histogram)
	{
		_prevAdaptedExposureScaleRef.swap(_adaptedExposureScaleRef);

		float histogramIllumScale = 1.0f / (histogramMax - histogramMin);
		float histogramIllumOffset = -histogramMin * histogramIllumScale;

		auto ps = Shader::FindOrCreate<EyeAdaptionPS>();
		ps->SetScalar("histogramIllumScale", histogramIllumScale);
		ps->SetScalar("histogramIllumOffset", histogramIllumOffset);
		ps->SetScalar("lowPercent", 80.0f / 100.0f);
		ps->SetScalar("highPercent", 99.0f / 100.0f);
		ps->SetScalar("frameTime", Global::GetInfo()->elapsedTime);
		ps->SetScalar("exposureScale", 0.3f);
		ps->SetScalar("eyeAdaptionMin", 0.04f);
		ps->SetScalar("eyeAdaptionMax", 2.2f);
		ps->SetSRV("histogram", histogram->GetShaderResourceView());
		ps->SetSRV("prevAdaptedExposureScale", _prevAdaptedExposureScaleRef->Get()->Cast<Texture>()->GetShaderResourceView());
		ps->Flush();

		DrawQuad({ _adaptedExposureScaleRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });
	}
}