#pragma once
#ifndef D3D11TEXTURE2D_H
#define D3D11TEXTURE2D_H

#include "ToyGE\D3D11\D3D11Texture.h"
#include "ToyGE\Kernel\Global.h"

namespace ToyGE
{
	class D3D11Texture2D : public D3D11Texture
	{
	public:
		D3D11Texture2D(const TextureDesc & desc);
		D3D11Texture2D(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList);

		static Ptr<D3D11Texture2D>
			CreateFromRawD3D(
			const Ptr<ID3D11Device> & rawDevice, const Ptr<ID3D11Texture2D> & rawTexture2D);

		Ptr<ID3D11Resource> RawD3DTexture() override
		{
			return _rawD3DTexture2D;
		}

		const Ptr<ID3D11ShaderResourceView>& 
			AcquireRawD3DShaderResourceView
			(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

		const Ptr<ID3D11RenderTargetView>& 
			AcquireRawD3DRenderTargetView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

		const Ptr<ID3D11DepthStencilView>& 
			AcquireRawD3DDepthStencilView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

		const Ptr<ID3D11UnorderedAccessView>& 
			AcquireRawD3DUnorderedAccessView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

	private:
		Ptr<ID3D11Texture2D> _rawD3DTexture2D;

		D3D11Texture2D() = default;
		void InitFromRawD3DTexture();
		void CreateRawD3DTexture2D_Desc(bool hasInitData, D3D11_TEXTURE2D_DESC & outDesc);
	};
}

#endif