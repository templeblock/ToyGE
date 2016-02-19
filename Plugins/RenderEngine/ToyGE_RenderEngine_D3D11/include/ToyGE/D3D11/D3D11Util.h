#pragma once
#ifndef D3D11RENDERFORMAT_H
#define D3D11RENDERFORMAT_H

#include "ToyGE\D3D11\D3D11PreInclude.h"
#include "ToyGE\RenderEngine\RenderFormat.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\RenderEngine\BlendState.h"

namespace ToyGE
{
	inline DXGI_FORMAT GetD3DFormat(RenderFormat format)
	{
		return static_cast<DXGI_FORMAT>(format);
	}

	inline RenderFormat GetRenderFormat(DXGI_FORMAT format)
	{
		return static_cast<RenderFormat>(format);
	}

	D3D11_COMPARISON_FUNC GetD3DComparisonFunc(ComparisonFunc func);

	D3D11_BLEND GetD3DBlend(BlendParam blend);

	D3D11_BLEND_OP GetD3DBlendOP(BlendOperation blendOP);

	uint8_t GetD3DColorWriteMask(uint8_t mask);

	D3D11_DEPTH_WRITE_MASK GetD3DDepthWriteMask(DepthWriteMask mask);

	D3D11_STENCIL_OP GetD3DStencilOP(StencilOperation op);

	D3D11_FILL_MODE GetD3DFillMode(FillMode mode);

	D3D11_CULL_MODE GetD3DCullMode(CullMode mode);

	D3D11_FILTER GetD3DFilter(Filter filter);

	D3D11_TEXTURE_ADDRESS_MODE GetD3DTextureAddressMode(TextureAddressMode mode);

	D3D11_PRIMITIVE_TOPOLOGY GetD3DPrimitiveTopology(PrimitiveTopology topology);
}

#endif