#pragma once
#ifndef D3D11DEPTHSTENCILSTATE_H
#define D3D11DEPTHSTENCILSTATE_H

#include "ToyGE\RenderEngine\DepthStencilState.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11DepthStencilState : public DepthStencilState
	{
	public:
		D3D11DepthStencilState(const DepthStencilStateDesc & desc);

		const Ptr<ID3D11DepthStencilState> & RawD3DDepthStencilState() const
		{
			return _rawD3DDepthStencilState;
		}

	private:
		Ptr<ID3D11DepthStencilState> _rawD3DDepthStencilState;

		void CreateD3DDepthStencilDesc(const DepthStencilStateDesc & desc, D3D11_DEPTH_STENCIL_DESC & d3dDesc);
	};
}

#endif