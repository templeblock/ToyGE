#pragma once
#ifndef D3D11TEXTURE3D_H
#define D3D11TEXTURE3D_H

#include "ToyGE\D3D11\D3D11Texture.h"

namespace ToyGE
{
	class D3D11Texture3D : public D3D11Texture
	{
	public:
		D3D11Texture3D(const TextureDesc & desc);
		D3D11Texture3D(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList);

		static Ptr<D3D11Texture3D>
			CreateFromRawD3D(
			const Ptr<ID3D11Device> & rawDevice, const Ptr<ID3D11Texture3D> & rawTexture3D);

		Ptr<ID3D11Resource> RawD3DTexture() override
		{
			return _rawD3DTexture3D;
		}

		const Ptr<ID3D11ShaderResourceView>& 
			AcquireRawD3DShaderResourceView
			(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

		const Ptr<ID3D11RenderTargetView>& 
			AcquireRawD3DRenderTargetView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

		const Ptr<ID3D11UnorderedAccessView>& 
			AcquireRawD3DUnorderedAccessView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) override;

	private:
		Ptr<ID3D11Texture3D> _rawD3DTexture3D;

		D3D11Texture3D() = default;
		void InitFromRawD3DTexture();
		void CreateRawD3DTexture3D_Desc(bool hasInitData, D3D11_TEXTURE3D_DESC & outDesc);
	};
}

#endif