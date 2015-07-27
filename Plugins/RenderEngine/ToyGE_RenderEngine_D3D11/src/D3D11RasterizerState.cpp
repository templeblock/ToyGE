#include "ToyGE\D3D11\D3D11RasterizerState.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	D3D11RasterizerState::D3D11RasterizerState(const RasterizerStateDesc & desc)
		: RasterizerState(desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_RASTERIZER_DESC d3dRasterizerDesc;
		CreateD3DRasterizerStateDesc(desc, d3dRasterizerDesc);
		ID3D11RasterizerState *pD3DRasterizerState = nullptr;
		re->RawD3DDevice()->CreateRasterizerState(&d3dRasterizerDesc, &pD3DRasterizerState);
		_rawD3DRasterizerState = MakeComShared(pD3DRasterizerState);
	}

	void D3D11RasterizerState::CreateD3DRasterizerStateDesc(const RasterizerStateDesc & desc, D3D11_RASTERIZER_DESC & d3dDesc)
	{
		d3dDesc.FillMode = GetD3DFillMode(desc.fillMode);
		d3dDesc.CullMode = GetD3DCullMode(desc.cullMode);
		d3dDesc.FrontCounterClockwise = desc.bFrontCounterClockwise;
		d3dDesc.DepthBias = desc.depthBias;
		d3dDesc.DepthBiasClamp = desc.depthBiasClamp;
		d3dDesc.SlopeScaledDepthBias = desc.slopeScaledDepthBias;
		d3dDesc.DepthClipEnable = desc.depthClipEnable;
		d3dDesc.ScissorEnable = desc.scissorEnable;
		d3dDesc.MultisampleEnable = desc.multisampleEnable;
		d3dDesc.AntialiasedLineEnable = desc.antialiasedLineEnable;
	}
}