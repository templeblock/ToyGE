#pragma once
#ifndef D3D11DEPTHSTENCILSTATE_H
#define D3D11DEPTHSTENCILSTATE_H

#include "ToyGE\D3D11\D3D11PreInclude.h"
#include "ToyGE\RenderEngine\DepthStencilState.h"

namespace ToyGE
{
	class D3D11DepthStencilState : public DepthStencilState
	{
	public:
		virtual void Init() override;

		virtual void Release() override
		{
			DepthStencilState::Release();
			_hardwareDepthStencilState = nullptr;
		}

		const Ptr<ID3D11DepthStencilState> & GetHardwareDepthStencilState() const
		{
			return _hardwareDepthStencilState;
		}

	private:
		Ptr<ID3D11DepthStencilState> _hardwareDepthStencilState;
	};
}

#endif