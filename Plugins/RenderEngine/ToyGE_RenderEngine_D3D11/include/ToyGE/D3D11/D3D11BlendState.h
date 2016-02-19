#pragma once
#ifndef D3D11BLENDSTATE_H
#define D3D11BLENDSTATE_H

#include "ToyGE\D3D11\D3D11PreInclude.h"
#include "ToyGE\RenderEngine\BlendState.h"

namespace ToyGE
{
	class D3D11BlendState : public BlendState
	{
	public:
		virtual void Init() override;

		virtual void Release() override
		{
			BlendState::Release();
			_hardwareBlendState = nullptr;
		}

		const Ptr<ID3D11BlendState> & GetHardwareBlendState() const
		{
			return _hardwareBlendState;
		}

	private:
		Ptr<ID3D11BlendState> _hardwareBlendState;
	};
}

#endif