#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	D3D11Sampler::D3D11Sampler(const SamplerDesc & desc)
		: Sampler(desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_SAMPLER_DESC d3dSamplerDesc;
		CreateD3DSamplerDesc(desc, d3dSamplerDesc);
		ID3D11SamplerState *pD3DSampler = nullptr;
		re->RawD3DDevice()->CreateSamplerState(&d3dSamplerDesc, &pD3DSampler);
		_rawD3DSampler = MakeComShared(pD3DSampler);
	}

	void D3D11Sampler::CreateD3DSamplerDesc(const SamplerDesc & desc, D3D11_SAMPLER_DESC & d3dDesc)
	{
		d3dDesc.Filter = GetD3DFilter(desc.filter);
		d3dDesc.AddressU = GetD3DTextureAddressMode(desc.addressU);
		d3dDesc.AddressV = GetD3DTextureAddressMode(desc.addressV);
		d3dDesc.AddressW = GetD3DTextureAddressMode(desc.addressW);
		d3dDesc.MipLODBias = desc.mipLODBias;
		d3dDesc.MaxAnisotropy = desc.maxAnisotropy;
		d3dDesc.ComparisonFunc = GetD3DComparisonFunc(desc.comparisonFunc);
		std::copy(desc.borderColor, desc.borderColor + 4, d3dDesc.BorderColor);
		d3dDesc.MinLOD = desc.minLOD;
		d3dDesc.MaxLOD = desc.maxLOD;
	}
}