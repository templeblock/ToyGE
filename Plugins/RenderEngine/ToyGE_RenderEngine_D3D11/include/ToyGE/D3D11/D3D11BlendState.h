#pragma once
#ifndef D3D11BLENDSTATE_H
#define D3D11BLENDSTATE_H

#include "ToyGE\RenderEngine\BlendState.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11BlendState : public BlendState
	{
	public:
		D3D11BlendState(const BlendStateDesc & desc);

		const Ptr<ID3D11BlendState> & RawD3DBlendState() const
		{
			return _rawD3DBlendState;
		}

	private:
		Ptr<ID3D11BlendState> _rawD3DBlendState;

		void CreateD3DBlendStateDesc(const BlendStateDesc & desc, D3D11_BLEND_DESC & d3dBlendStateDesc);
	};
}

#endif