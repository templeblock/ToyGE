#pragma once
#ifndef D3D11RASTERIZERSTATE_H
#define D3D11RASTERIZERSTATE_H

#include "ToyGE\RenderEngine\RasterizerState.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11RasterizerState : public RasterizerState
	{
	public:
		D3D11RasterizerState(const RasterizerStateDesc & desc);

		const Ptr<ID3D11RasterizerState> & RawD3DRasterizerState() const
		{
			return _rawD3DRasterizerState;
		}

	private:
		Ptr<ID3D11RasterizerState> _rawD3DRasterizerState;

		void CreateD3DRasterizerStateDesc(const RasterizerStateDesc & desc, D3D11_RASTERIZER_DESC & d3dDesc);
	};
}

#endif