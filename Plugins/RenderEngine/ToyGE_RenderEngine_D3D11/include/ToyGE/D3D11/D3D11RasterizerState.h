#pragma once
#ifndef D3D11RASTERIZERSTATE_H
#define D3D11RASTERIZERSTATE_H

#include "ToyGE\RenderEngine\RasterizerState.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{
	class D3D11RasterizerState : public RasterizerState
	{
	public:
		virtual void Init() override;

		virtual void Release() override
		{
			RasterizerState::Release();
			_hardwareD3DRasterizerState = nullptr;
		}

		const Ptr<ID3D11RasterizerState> & GetHardwareRasterizerState() const
		{
			return _hardwareD3DRasterizerState;
		}

	private:
		Ptr<ID3D11RasterizerState> _hardwareD3DRasterizerState;
	};
}

#endif