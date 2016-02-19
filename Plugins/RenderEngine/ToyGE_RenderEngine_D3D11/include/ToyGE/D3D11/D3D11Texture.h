#pragma once
#ifndef D3D11TEXTURE_H
#define D3D11TEXTURE_H

#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\D3D11\D3D11ResourceView.h"

namespace ToyGE
{
	// D3D11 views
	class D3D11TextureShaderResourceView : public TextureShaderResourceView, public D3D11ShaderResourceView
	{
	public:
		bool bCube = false;

		virtual ~D3D11TextureShaderResourceView() = default;

		virtual void Release() override
		{
			hardwareSRV = nullptr;
		}
	};

	class D3D11TextureUnorderedAccessView : public TextureUnorderedAccessView, public D3D11UnorderedAccessView
	{
	public:
		virtual ~D3D11TextureUnorderedAccessView() = default;

		virtual void Release() override
		{
			hardwareUAV = nullptr;
		}
	};

	class D3D11TextureRenderTargetView : public TextureRenderTargetView, public D3D11RenderTargetView
	{
	public:
		virtual ~D3D11TextureRenderTargetView() = default;

		virtual void Release() override
		{
			hardwareRTV = nullptr;
		}
	};

	class D3D11TextureDepthStencilView : public TextureDepthStencilView, public D3D11DepthStencilView
	{
	public:
		virtual ~D3D11TextureDepthStencilView() = default;

		virtual void Release() override
		{
			hardwareDSV = nullptr;
		}
	};

	// D3D11 texture
	class D3D11Texture : public Texture
	{
	public:
		D3D11Texture(TextureType type) : Texture(type) {}

		virtual void InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource) = 0;

		bool GenerateMips() override;

		bool CopyTo(const Ptr<Texture> & dst,
			int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
			int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox = std::shared_ptr<Box>()) const override;

		RenderDataDesc Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex) override;

		void UnMap() override;

		virtual Ptr<ID3D11Resource> GetHardwareTexture() const = 0;

	protected:
		int32_t _mappedSubResource = -1;

		virtual Ptr<TextureShaderResourceView> CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat) override { return nullptr; };

		virtual Ptr<TextureUnorderedAccessView> CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override { return nullptr; };

		virtual Ptr<TextureRenderTargetView> CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override { return nullptr; };

		virtual Ptr<TextureDepthStencilView> CreateDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) override { return nullptr; };

		void GetD3DTextureCreateFlags(bool bWithInitData, uint32_t & d3dBindFlags, uint32_t & d3dCpuAccessFlags, uint32_t & d3dMiscFlags, D3D11_USAGE & d3dUsage);

		void GetFlagsFromD3D(uint32_t d3dBindFlags, uint32_t d3dCpuAccessFlags, uint32_t d3dMiscFlags, uint32_t & outBindFlags, uint32_t & outCpuAccess, uint32_t & bCube);
	};
}

#endif