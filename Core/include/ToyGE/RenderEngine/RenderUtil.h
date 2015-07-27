#pragma once
#ifndef RENDERUTIL_H
#define RENDERUTIL_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"

namespace ToyGE
{
	class Texture;
	class RenderTechnique;

	TOYGE_CORE_API uint32_t GetRenderFormatNumBits(RenderFormat fmt);

	TOYGE_CORE_API bool IsCompress(RenderFormat fmt);

	template <typename T>
	bool CheckRenderConfig(const Ptr<RenderSharedEnviroment> & sharedEnviron, const String & paramName, const T & value)
	{
		auto param = sharedEnviron->ParamByName(paramName);
		if (param && param->As<SharedParam<T>>()->GetValue() == value)
			return true;
		return false;
	}

	TOYGE_CORE_API Ptr<Texture> HeightToNormal(const Ptr<Texture> & heightTex);

	TOYGE_CORE_API Ptr<Texture> SpecularToRoughness(const Ptr<Texture> & shininessTex);

	TOYGE_CORE_API void Transform(
		const ResourceView & src,
		const ResourceView & dst,
		//uint32_t colorMask = COLOR_WRITE_ALL,
		const Vector4<ColorWriteMask> & colorWriteMask = Vector4<ColorWriteMask>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A),
		const int4 & dstRect = -1);
	
	TOYGE_CORE_API Ptr<Texture> SAT(const Ptr<Texture> & tex);

	TOYGE_CORE_API Ptr<Texture> DownSample(const ResourceView & texView, float2 scale);

	TOYGE_CORE_API void RenderQuad(
		const Ptr<RenderTechnique> & tech,
		int32_t topLeftX = 0,
		int32_t topLeftY = 0,
		int32_t width = 0,
		int32_t height = 0,
		const ResourceView & depthStencil = ResourceView());

	TOYGE_CORE_API void TextureFilter(
		const Ptr<Texture> & src,
		int32_t srcMipLevel,
		int32_t srcArrayOffset,
		const Ptr<Texture> & dst,
		int32_t dstMipLevel,
		int32_t dstArrayOffset,
		int32_t numSamples,
		const std::vector<float2> & uvOffsets, 
		const std::vector<float> & weights);

	TOYGE_CORE_API void BilateralUpSampling(
		const ResourceView & src,
		const ResourceView & lowResDepthTex,
		const ResourceView & highResDepthTex,
		const ResourceView & dst,
		float depthDiffScale);
}
#endif