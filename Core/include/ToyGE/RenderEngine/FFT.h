#pragma once
#ifndef FFT_H
#define FFT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	DECLARE_SHADER(, FFTRadix2_2D_XCS, SHADER_CS, "FFT", "FFTRadix2_2D_XCS", SM_5);
	DECLARE_SHADER(, FFTRadix2_2D_YCS, SHADER_CS, "FFT", "FFTRadix2_2D_YCS", SM_5);
	DECLARE_SHADER(, FFTRadix2_1DCS, SHADER_CS, "FFT", "FFTRadix2_1DCS", SM_5);

	DECLARE_SHADER(, FFTRadix8_2D_XCS, SHADER_CS, "FFT", "FFTRadix8_2D_XCS", SM_5);
	DECLARE_SHADER(, FFTRadix8_2D_X_FinalCS, SHADER_CS, "FFT", "FFTRadix8_2D_X_FinalCS", SM_5);
	DECLARE_SHADER(, FFTRadix8_2D_YCS, SHADER_CS, "FFT", "FFTRadix8_2D_YCS", SM_5);
	DECLARE_SHADER(, FFTRadix8_2D_Y_FinalCS, SHADER_CS, "FFT", "FFTRadix8_2D_Y_FinalCS", SM_5);
	DECLARE_SHADER(, FFTRadix8_1DCS, SHADER_CS, "FFT", "FFTRadix8_1DCS", SM_5);
	DECLARE_SHADER(, FFTRadix8_1D_FinalCS, SHADER_CS, "FFT", "FFTRadix8_1D_FinalCS", SM_5);

	DECLARE_SHADER(, FFTSetupPS, SHADER_PS, "FFTUtil", "FFTSetupPS", SM_4);
	DECLARE_SHADER(, IFFTScalePS, SHADER_PS, "FFTUtil", "IFFTScalePS", SM_4);
	DECLARE_SHADER(, IFFTConjugatePS, SHADER_PS, "FFTUtil", "IFFTConjugatePS", SM_4);
	DECLARE_SHADER(, ShowFFTPS, SHADER_PS, "FFTUtil", "ShowFFTPS", SM_4);

	class Texture;
	class ShaderResourceView;
	class RenderTargetView;

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
			bool bInverse = false,
			bool bIFFTScale = true);

		static void FFT2DRadix8(
			const Ptr<Texture> & src,
			int32_t srcMipLevel,
			int32_t srcArrayOffset,
			const Ptr<Texture> & dst,
			int32_t dstMipLevel,
			int32_t dstArrayOffset,
			bool bInverse = false,
			bool bIFFTScale = true);

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
