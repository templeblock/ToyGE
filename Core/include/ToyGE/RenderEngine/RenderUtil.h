#pragma once
#ifndef RENDERUTIL_H
#define RENDERUTIL_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;
	class RenderTechnique;
	class RenderView;
	class RenderTargetView;
	class DepthStencilView;

	TOYGE_CORE_API uint32_t GetRenderFormatNumBits(RenderFormat fmt);

	TOYGE_CORE_API bool IsCompress(RenderFormat fmt);

	TOYGE_CORE_API void ComputeMipsSize(
		int32_t width,
		int32_t heigth,
		int32_t depth,
		std::vector<int3> & outMipsSize);

	TOYGE_CORE_API const std::vector<float> & GetGaussTable(int32_t numSamples);

	TOYGE_CORE_API RenderViewport GetTextureQuadViewport(const Ptr<Texture> & tex);

	TOYGE_CORE_API RenderViewport GetQuadViewport();


	DECLARE_SHADER(, HeightToBumpPS, SHADER_PS, "HeightToBump", "HeightToBumpPS", SM_4);

	TOYGE_CORE_API Ptr<Texture> HeightToBumpTex(const Ptr<Texture> & heightTex, float scale = 1.0f);


	DECLARE_SHADER(, DrawQuadVS, SHADER_VS, "DrawQuad", "DrawQuadVS", SM_4);

	TOYGE_CORE_API void DrawQuad(
		const std::vector< Ptr<RenderTargetView> > & rtvs,
		float topLeftX = 0.0f,
		float topLeftY = 0.0f,
		float width = 0.0f,
		float height = 0.0f,
		float topLeftU = 0.0f,
		float topLeftV = 0.0f,
		float uvWidth = 1.0f,
		float uvHeight = 1.0f,
		const Ptr<DepthStencilView> & dsv = nullptr);


	DECLARE_SHADER(, RenderToVolumeTextureVS, SHADER_VS, "RenderToVolumeTexture", "RenderToVolumeTextureVS", SM_4);
	DECLARE_SHADER(, RenderToVolumeTextureGS, SHADER_GS, "RenderToVolumeTexture", "RenderToVolumeTextureGS", SM_4);

	TOYGE_CORE_API void RenderToVolumeTexture(const Ptr<Texture> & volumeTexture);

	DECLARE_SHADER(, FillPS, SHADER_PS, "FillPS", "FillPS", SM_4);

	TOYGE_CORE_API void Fill(
		const float4 & color,
		const std::vector< Ptr<RenderTargetView> > & rtvs,
		float topLeftX = 0.0f,
		float topLeftY = 0.0f,
		float width = 0.0f,
		float height = 0.0f,
		float topLeftU = 0.0f,
		float topLeftV = 0.0f,
		float uvWidth = 1.0f,
		float uvHeight = 1.0f,
		const Ptr<DepthStencilView> & dsv = nullptr);


	DECLARE_SHADER(, TransformPS, SHADER_PS, "TransformPS", "TransformPS", SM_4);

	TOYGE_CORE_API void Transform(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const Vector<ColorWriteMask, 4> & colorWriteMask = Vector<ColorWriteMask, 4>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A),
		const float4 & srcRect = 0.0f,
		const float4 & dstRect = 0.0f,
		const Ptr<class Sampler> & sampler = nullptr,
		const Ptr<DepthStencilView> & dsv = nullptr);


	DECLARE_SHADER(, FilterVS, SHADER_VS, "FilterVS", "FilterVS", SM_4);
	DECLARE_SHADER(, FilterPS, SHADER_PS, "FilterPS", "FilterPS", SM_4);

	TOYGE_CORE_API void TextureFilter(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const std::vector<float2> & uvOffsets, 
		const std::vector<float> & weights,
		const Ptr<class Sampler> & sampler = nullptr);


	DECLARE_SHADER(, BilateralBlurXPS, SHADER_PS, "BilateralFilter", "BilateralBlurXPS", SM_4);
	DECLARE_SHADER(, BilateralBlurYPS, SHADER_PS, "BilateralFilter", "BilateralBlurYPS", SM_4);

	TOYGE_CORE_API void BilateralBlur(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst, 
		const Ptr<ShaderResourceView> & depthTex,
		const std::vector<float> & weights,
		float depthDiffThreshold);


	DECLARE_SHADER(, BilateralUpSamplingPS, SHADER_PS, "BilateralFilter", "BilateralUpSamplingPS", SM_4);

	TOYGE_CORE_API void BilateralUpSampling(
		const Ptr<ShaderResourceView> & src,
		const Ptr<RenderTargetView> & dst,
		const Ptr<ShaderResourceView> & lowResDepthTex,
		const Ptr<ShaderResourceView> & highResDepthTex,
		float depthDiffThreshold);


	DECLARE_SHADER(, LinearizeDepthPS, SHADER_PS, "LinearizeDepth", "LinearizeDepthPS", SM_4);

	TOYGE_CORE_API void LinearizeDepth(const Ptr<ShaderResourceView> & depth, const Ptr<RenderView> & view, const Ptr<RenderTargetView> & target);


	DECLARE_SHADER(, TemporalAAPS, SHADER_PS, "TemporalAA", "TemporalAAPS", SM_4);


	DECLARE_SHADER(, CubeMapToPanoramicPS, SHADER_PS, "AmbientMapTransform", "CubeMapToPanoramicPS", SM_4);

	TOYGE_CORE_API void CubeMapToPnoramic(const Ptr<Texture> & cubeMap, const Ptr<Texture> & targetPanoramic);

	DECLARE_SHADER(, CubeMapToHemiPanoramicPS, SHADER_PS, "AmbientMapTransform", "CubeMapToHemiPanoramicPS", SM_4);

	TOYGE_CORE_API void CubeMapToHemiPnoramic(const Ptr<Texture> & cubeMap, const Ptr<Texture> & targetPanoramic);
}
#endif