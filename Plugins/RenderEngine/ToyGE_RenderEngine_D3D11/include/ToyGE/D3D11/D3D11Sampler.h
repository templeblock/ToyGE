#pragma once
#ifndef D3D11SAMPLER_H
#define D3D11SAMPLER_H

#include "ToyGE\RenderEngine\Sampler.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11Sampler : public Sampler
	{
	public:
		D3D11Sampler(const SamplerDesc & desc);

		const Ptr<ID3D11SamplerState> & RawD3DSampler() const
		{
			return _rawD3DSampler;
		}

	private:
		Ptr<ID3D11SamplerState> _rawD3DSampler;

		void CreateD3DSamplerDesc(const SamplerDesc & desc, D3D11_SAMPLER_DESC & d3dDesc);
	};
}

#endif