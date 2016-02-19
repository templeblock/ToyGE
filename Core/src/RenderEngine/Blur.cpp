#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"

namespace ToyGE
{
	void Blur::BoxBlur(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int2 numSamples,
		float2 blurRadius)
	{
		auto dstTex = dst->GetResource()->Cast<Texture>();
		auto mipSize = dstTex->GetMipSize(dst->Cast<TextureRenderTargetView>()->mipLevel);

		TextureDesc texDesc = dst->GetResource()->Cast<Texture>()->GetDesc();
		texDesc.width = mipSize.x();
		texDesc.height = mipSize.y();
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		auto tmpTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		BoxBlurX(src, tmpTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), numSamples.x(), blurRadius.x());

		BoxBlurY(tmpTex->Get()->Cast<Texture>()->GetShaderResourceView(0, 1, 0, 1), dst, numSamples.y(), blurRadius.y());
	}

	void Blur::BoxBlurX(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius)
	{
		float w = 1.0f / (float)numSamples;
		std::vector<float> weights(numSamples, w);

		BlurX(src, dst, numSamples, blurRadius, weights);
	}

	void Blur::BoxBlurY(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius)
	{
		float w = 1.0f / (float)numSamples;
		std::vector<float> weights(numSamples, w);

		BlurY(src, dst, numSamples, blurRadius, weights);
	}

	void Blur::GaussBlur(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int2 numSamples,
		float2 blurRadius)
	{
		auto dstTex = dst->GetResource()->Cast<Texture>();
		auto mipSize = dstTex->GetMipSize(dst->Cast<TextureRenderTargetView>()->mipLevel);

		TextureDesc texDesc = dst->GetResource()->Cast<Texture>()->GetDesc();
		texDesc.width = mipSize.x();
		texDesc.height = mipSize.y();
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		auto tmpTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		GaussBlurX(src, tmpTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), numSamples.x(), blurRadius.x());

		GaussBlurY(tmpTex->Get()->Cast<Texture>()->GetShaderResourceView(0, 1, 0, 1), dst, numSamples.y(), blurRadius.y());
	}

	void Blur::GaussBlurX(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius)
	{
		BlurX(src, dst, numSamples, blurRadius, GetGaussTable(numSamples));
	}

	void Blur::GaussBlurY(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius)
	{
		BlurY(src, dst, numSamples, blurRadius, GetGaussTable(numSamples));
	}

	void Blur::BlurX(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius,
		const std::vector<float> & weights)
	{
		auto srcTex = src->GetResource()->Cast<Texture>();

		auto & mipSize = srcTex->GetMipSize(src->Cast<TextureShaderResourceView>()->firstMip);
		float2 texelSize = 1.0f / float2((float)mipSize.x(), (float)mipSize.y());

		float offsetStep = 0.0f;
		float offsetStart = 0.0f;
		if (numSamples > 0)
		{
			offsetStep = blurRadius / (float)(numSamples - 1);
			offsetStart = -blurRadius * 0.5f;
		}

		std::vector<float2> offsets;
		for (int32_t i = 0; i < numSamples; ++i)
			offsets.push_back(float2((offsetStart + (float)i * offsetStep)  * texelSize.x(), 0.0f));

		TextureFilter(src, dst, offsets, weights);
	}

	void Blur::BlurY(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		int32_t numSamples,
		float blurRadius,
		const std::vector<float> & weights)
	{
		auto srcTex = src->GetResource()->Cast<Texture>();

		auto & mipSize = srcTex->GetMipSize(src->Cast<TextureShaderResourceView>()->firstMip);
		float2 texelSize = 1.0f / float2((float)mipSize.x(), (float)mipSize.y());

		float offsetStep = 0.0f;
		float offsetStart = 0.0f;
		if (numSamples > 0)
		{
			offsetStep = blurRadius / (float)(numSamples - 1);
			offsetStart = -blurRadius * 0.5f;
		}

		std::vector<float2> offsets;
		for (int32_t i = 0; i < numSamples; ++i)
			offsets.push_back(float2(0.0f, (offsetStart + (float)i * offsetStep)  * texelSize.y()));

		TextureFilter(src, dst, offsets, weights);
	}
}