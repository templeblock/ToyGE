#include "ToyGE\D3D11\D3D11DepthStencilState.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	D3D11DepthStencilState::D3D11DepthStencilState(const DepthStencilStateDesc & desc)
		: DepthStencilState(desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
		CreateD3DDepthStencilDesc(desc, d3dDepthStencilDesc);
		ID3D11DepthStencilState *pD3DDepthStecilState = nullptr;
		re->RawD3DDevice()->CreateDepthStencilState(&d3dDepthStencilDesc, &pD3DDepthStecilState);
		_rawD3DDepthStencilState = MakeComShared(pD3DDepthStecilState);
	}

	void D3D11DepthStencilState::CreateD3DDepthStencilDesc(const DepthStencilStateDesc & desc, D3D11_DEPTH_STENCIL_DESC & d3dDesc)
	{
		d3dDesc.DepthEnable = desc.depthEnable;
		d3dDesc.DepthWriteMask = GetD3DDepthWriteMask(desc.depthWriteMask);
		d3dDesc.DepthFunc = GetD3DComparisonFunc(desc.depthFunc);
		d3dDesc.StencilEnable = desc.stencilEnable;
		d3dDesc.StencilReadMask = desc.stencilReadMask;
		d3dDesc.StencilWriteMask = desc.stencilWriteMask;

		d3dDesc.BackFace.StencilFailOp = GetD3DStencilOP(desc.backFace.stencilFailOp);
		d3dDesc.BackFace.StencilDepthFailOp = GetD3DStencilOP(desc.backFace.stencilDepthFailOp);
		d3dDesc.BackFace.StencilPassOp = GetD3DStencilOP(desc.backFace.stencilPassOp);
		d3dDesc.BackFace.StencilFunc = GetD3DComparisonFunc(desc.backFace.stencilFunc);

		d3dDesc.FrontFace.StencilFailOp = GetD3DStencilOP(desc.frontFace.stencilFailOp);
		d3dDesc.FrontFace.StencilDepthFailOp = GetD3DStencilOP(desc.frontFace.stencilDepthFailOp);
		d3dDesc.FrontFace.StencilPassOp = GetD3DStencilOP(desc.frontFace.stencilPassOp);
		d3dDesc.FrontFace.StencilFunc = GetD3DComparisonFunc(desc.frontFace.stencilFunc);
		
	}
}