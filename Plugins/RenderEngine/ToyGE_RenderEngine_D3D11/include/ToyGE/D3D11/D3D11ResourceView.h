#pragma once
#ifndef D3D11RESOURCEVIEW_H
#define D3D11RESOURCEVIEW_H

#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{

	class D3D11ShaderResourceView
	{
	public:
		Ptr<ID3D11ShaderResourceView> hardwareSRV;
	};

	class D3D11UnorderedAccessView
	{
	public:
		Ptr<ID3D11UnorderedAccessView> hardwareUAV;
	};

	class D3D11RenderTargetView
	{
	public:
		Ptr<ID3D11RenderTargetView> hardwareRTV;
	};

	class D3D11DepthStencilView
	{
	public:
		Ptr<ID3D11DepthStencilView> hardwareDSV;
	};
}

#endif

