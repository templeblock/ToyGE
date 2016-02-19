#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	static void CreateD3DSamplerDesc(const SamplerDesc & desc, D3D11_SAMPLER_DESC & d3dDesc)
	{
		d3dDesc.Filter = GetD3DFilter(desc.filter);
		d3dDesc.AddressU = GetD3DTextureAddressMode(desc.addressU);
		d3dDesc.AddressV = GetD3DTextureAddressMode(desc.addressV);
		d3dDesc.AddressW = GetD3DTextureAddressMode(desc.addressW);
		d3dDesc.MipLODBias = desc.mipLODBias;
		d3dDesc.MaxAnisotropy = desc.maxAnisotropy;
		d3dDesc.ComparisonFunc = GetD3DComparisonFunc(desc.comparisonFunc);
		d3dDesc.BorderColor[0] = desc.borderColor.x();
		d3dDesc.BorderColor[1] = desc.borderColor.y();
		d3dDesc.BorderColor[2] = desc.borderColor.z();
		d3dDesc.BorderColor[3] = desc.borderColor.w();
		d3dDesc.MinLOD = desc.minLOD;
		d3dDesc.MaxLOD = desc.maxLOD;
	}

	void D3D11Sampler::Init()
	{
		Sampler::Init();

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_SAMPLER_DESC d3dSamplerDesc;
		CreateD3DSamplerDesc(_desc, d3dSamplerDesc);

		ID3D11SamplerState *pD3DSampler = nullptr;
		D3D11RenderEngine::d3d11Device->CreateSamplerState(&d3dSamplerDesc, &pD3DSampler);
		_hardwareSampler = MakeComShared(pD3DSampler);
	}

	
}