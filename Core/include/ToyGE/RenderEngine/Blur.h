#pragma once
#ifndef BLUR_H
#define BLUR_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;

	class TOYGE_CORE_API Blur
	{
	public:
		static const std::vector<float> & GaussTable(int32_t blurRadius);

		static void BoxBlur(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float2 sampleOffsetScale);

		static void BoxBlurX(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale);

		static void BoxBlurY(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale);

		static void GaussBlur(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float2 sampleOffsetScale);

		static void GaussBlurX(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale);

		static void GaussBlurY(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale);


	private:
		static std::vector< std::vector<float> > _gaussTableMap;

		static void SeparableBlur(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float2 sampleOffsetScale,
			const std::vector<float> & weights);

		static void BlurX(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale,
			const std::vector<float> & weights);

		static void BlurY(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			int32_t blurRadius,
			float sampleOffsetScale,
			const std::vector<float> & weights);
	};
}

#endif