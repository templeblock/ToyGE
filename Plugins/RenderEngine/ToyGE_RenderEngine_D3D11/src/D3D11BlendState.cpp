#include "ToyGE\D3D11\D3D11BlendState.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	static void CreateD3DBlendStateDesc(const BlendStateDesc & desc, D3D11_BLEND_DESC & d3dBlendStateDesc)
	{
		d3dBlendStateDesc.AlphaToCoverageEnable = desc.alphaToCoverageEnable;
		d3dBlendStateDesc.IndependentBlendEnable = desc.independentBlendEnable;
		for (int32_t i = 0; i != 8; ++i)
		{
			auto & d3dRTDesc = d3dBlendStateDesc.RenderTarget[i];
			auto & rtDesc = desc.blendRTDesc[i];

			d3dRTDesc.BlendEnable = rtDesc.blendEnable;
			d3dRTDesc.SrcBlend = GetD3DBlend(rtDesc.srcBlend);
			d3dRTDesc.DestBlend = GetD3DBlend(rtDesc.dstBlend);
			d3dRTDesc.BlendOp = GetD3DBlendOP(rtDesc.blendOP);
			d3dRTDesc.SrcBlendAlpha = GetD3DBlend(rtDesc.srcBlendAlpha);
			d3dRTDesc.DestBlendAlpha = GetD3DBlend(rtDesc.dstBlendAlpha);
			d3dRTDesc.BlendOpAlpha = GetD3DBlendOP(rtDesc.blendOPAlpha);
			d3dRTDesc.RenderTargetWriteMask = GetD3DColorWriteMask(rtDesc.renderTargetWriteMask);
		}
	}

	void D3D11BlendState::Init()
	{
		BlendState::Init();

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		D3D11_BLEND_DESC d3dBlendStateDesc;
		CreateD3DBlendStateDesc(_desc, d3dBlendStateDesc);

		ID3D11BlendState *pD3DBlendState = nullptr;
		D3D11RenderEngine::d3d11Device->CreateBlendState(&d3dBlendStateDesc, &pD3DBlendState);
		_hardwareBlendState = MakeComShared(pD3DBlendState);
	}
}