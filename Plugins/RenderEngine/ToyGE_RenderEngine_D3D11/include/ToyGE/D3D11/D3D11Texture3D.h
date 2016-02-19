#pragma once
#ifndef D3D11TEXTURE3D_H
#define D3D11TEXTURE3D_H

#include "ToyGE\D3D11\D3D11Texture.h"

namespace ToyGE
{
	class D3D11Texture3D : public D3D11Texture
	{
	public:
		D3D11Texture3D() : D3D11Texture(TEXTURE_3D) {}

		virtual void Init(const std::vector<RenderDataDesc> & initDataList) override;

		virtual void InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource) override;

		virtual void Release() override
		{
			D3D11Texture::Release();
			_hardwareTexture3D = nullptr;
		}

		Ptr<ID3D11Resource> GetHardwareTexture() const override
		{
			return _hardwareTexture3D;
		}
	private:
		Ptr<ID3D11Texture3D> _hardwareTexture3D;

		virtual Ptr<TextureShaderResourceView> CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat) override;

		virtual Ptr<TextureUnorderedAccessView> CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override;

		virtual Ptr<TextureRenderTargetView> CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override;
	};
}

#endif