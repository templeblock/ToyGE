#pragma once
#ifndef D3D11SAMPLER_H
#define D3D11SAMPLER_H

#include "ToyGE\RenderEngine\Sampler.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{
	class D3D11Sampler : public Sampler
	{
	public:
		virtual void Init() override;

		virtual void Release() override
		{
			Sampler::Release();
			_hardwareSampler = nullptr;
		}

		const Ptr<ID3D11SamplerState> & GetHardwareSampler() const
		{
			return _hardwareSampler;
		}

	private:
		Ptr<ID3D11SamplerState> _hardwareSampler;
	};
}

#endif