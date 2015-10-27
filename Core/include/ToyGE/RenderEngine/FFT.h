#pragma once
#ifndef FFT_H
#define FFT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;

	class TOYGE_CORE_API FFT
	{
	public:
		static void FFT2DRadix2(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			bool bInverse = false);

		static void FFT2DRadix8(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			bool bInverse = false);

		static void FFTSetup(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset);
	};
}

#endif
