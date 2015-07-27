#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderTechnique.h"
#include "ToyGE\RenderEngine\RenderPass.h"
#include "ToyGE\RenderEngine\RenderEffectVariable.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	std::vector< std::vector<float> > Blur::_gaussTableMap;

	const std::vector<float> & Blur::GaussTable(int32_t blurRadius)
	{
		if (_gaussTableMap.size() < blurRadius + 1)
		{
			_gaussTableMap.resize(blurRadius + 1);
		}

		if (_gaussTableMap[blurRadius].size() == 0)
			GetGaussTable(blurRadius, _gaussTableMap[blurRadius]);

		return _gaussTableMap[blurRadius];
	}

	void Blur::BoxBlur(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float2 sampleOffsetScale)
	{
		float w = 1.0f / static_cast<float>(blurRadius * 2 + 1);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(w);

		SeparableBlur(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::BoxBlurX(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale)
	{
		float w = 1.0f / static_cast<float>(blurRadius * 2 + 1);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(w);

		BlurX(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::BoxBlurY(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale)
	{
		float w = 1.0f / static_cast<float>(blurRadius * 2 + 1);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(w);

		BlurY(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::GaussBlur(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float2 sampleOffsetScale)
	{
		auto & gaussTable = GaussTable(blurRadius);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(gaussTable[abs(i)]);

		SeparableBlur(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::GaussBlurX(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale)
	{
		auto & gaussTable = GaussTable(blurRadius);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(gaussTable[abs(i)]);

		BlurX(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::GaussBlurY(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale)
	{
		auto & gaussTable = GaussTable(blurRadius);
		std::vector<float> weights;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			weights.push_back(gaussTable[abs(i)]);

		BlurY(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale, weights);
	}

	void Blur::SeparableBlur(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float2 sampleOffsetScale,
		const std::vector<float> & weights)
	{
		auto & mipSize = src->GetMipSize(srcMipLevel);

		auto texDesc = src->Desc();
		texDesc.width = std::get<0>(mipSize);
		texDesc.height = std::get<1>(mipSize);
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		BlurX(src, srcMipLevel, srcArrayOffset, tmpTex, 0, 0, blurRadius, sampleOffsetScale.x, weights);

		BlurY(tmpTex, 0, 0, dst, dstMipLevel, dstArrayOffset, blurRadius, sampleOffsetScale.y, weights);

		tmpTex->Release();
	}

	void Blur::BlurX(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale,
		const std::vector<float> & weights)
	{
		auto & mipSize = src->GetMipSize(srcMipLevel);
		float2 texelSize = 1.0f / float2(static_cast<float>(std::get<0>(mipSize)), static_cast<float>(std::get<1>(mipSize)));

		std::vector<float2> offsets;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			offsets.push_back(float2(static_cast<float>(i)* texelSize.x * sampleOffsetScale, 0.0f));

		TextureFilter(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius * 2 + 1, offsets, weights);
	}

	void Blur::BlurY(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t blurRadius,
		float sampleOffsetScale,
		const std::vector<float> & weights)
	{
		auto & mipSize = src->GetMipSize(srcMipLevel);
		float2 texelSize = 1.0f / float2(static_cast<float>(std::get<0>(mipSize)), static_cast<float>(std::get<1>(mipSize)));

		std::vector<float2> offsets;
		for (int32_t i = -blurRadius; i <= blurRadius; ++i)
			offsets.push_back(float2(0.0f, static_cast<float>(i)* texelSize.y * sampleOffsetScale));

		TextureFilter(src, srcMipLevel, srcArrayOffset, dst, dstMipLevel, dstArrayOffset, blurRadius * 2 + 1, offsets, weights);
	}
}