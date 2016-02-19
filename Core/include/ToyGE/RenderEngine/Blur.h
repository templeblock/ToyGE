#pragma once
#ifndef BLUR_H
#define BLUR_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;
	class ShaderResourceView;
	class RenderTargetView;

	class TOYGE_CORE_API Blur
	{
	public:
		static void BoxBlur(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int2 numSamples,
			float2 blurRadius);

		static void BoxBlurX(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius);

		static void BoxBlurY(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius);

		static void GaussBlur(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int2 numSamples,
			float2 blurRadius);

		static void GaussBlurX(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius);

		static void GaussBlurY(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius);

		/*static void SeparableBlur(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int2 numSamples,
			float2 blurRadius,
			const std::vector<float> & weights);*/

		static void BlurX(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius,
			const std::vector<float> & weights);

		static void BlurY(
			const Ptr<ShaderResourceView> & src,
			const Ptr<RenderTargetView> & dst,
			int32_t numSamples,
			float blurRadius,
			const std::vector<float> & weights);
	};
}

#endif