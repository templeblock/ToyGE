#pragma once
#ifndef D3D11TEXTURE1D_H
#define D3D11TEXTURE1D_H

#include "ToyGE\D3D11\D3D11Texture.h"

namespace ToyGE
{
	class D3D11Texture1D : public D3D11Texture
	{
	public:
		D3D11Texture1D() : D3D11Texture(TEXTURE_1D) {}

		virtual void Init(const std::vector<RenderDataDesc> & initDataList) override;

		virtual void InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource) override;

		virtual void Release() override
		{
			D3D11Texture::Release();
			_hardwareTexture1D = nullptr;
		}

		Ptr<ID3D11Resource> GetHardwareTexture() const override
		{
			return _hardwareTexture1D;
		}

	protected:
		Ptr<ID3D11Texture1D> _hardwareTexture1D;

		virtual Ptr<TextureShaderResourceView> CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat) override;

		virtual Ptr<TextureUnorderedAccessView> CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override;

		virtual Ptr<TextureRenderTargetView> CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override;

		virtual Ptr<TextureDepthStencilView> CreateDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override;
	};
}

#endif