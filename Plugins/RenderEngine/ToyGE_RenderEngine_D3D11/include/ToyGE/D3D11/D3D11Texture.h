#pragma once
#ifndef D3D11TEXTURE_H
#define D3D11TEXTURE_H

#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11Texture : public Texture
	{
	public:
		D3D11Texture(const TextureDesc & desc);
		D3D11Texture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList);

		bool GenerateMips() override;

		bool CopyTo(const Ptr<Texture> & dst,
			int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
			int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox = std::shared_ptr<Box>()) const override;

		RenderDataDesc Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex) override;
		void UnMap() override;

		virtual Ptr<ID3D11Resource> RawD3DTexture() const = 0;

		virtual const Ptr<ID3D11ShaderResourceView>& 
			AcquireRawD3DShaderResourceView
			(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) = 0;
		virtual const Ptr<ID3D11ShaderResourceView>&
			AcquireRawD3DShaderResourceView_Cube
			(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstFaceOffset, int32_t numCubes, RenderFormat formatHint);

		virtual const Ptr<ID3D11RenderTargetView>&
			AcquireRawD3DRenderTargetView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) = 0;

		virtual const Ptr<ID3D11DepthStencilView>&
			AcquireRawD3DDepthStencilView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint);

		virtual const Ptr<ID3D11UnorderedAccessView>&
			AcquireRawD3DUnorderedAccessView
			(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint) = 0;

	protected:
		std::map<uint64_t, Ptr<ID3D11ShaderResourceView>> _srvMap;
		std::map<uint64_t, Ptr<ID3D11RenderTargetView>> _rtvMap;
		std::map<uint64_t, Ptr<ID3D11DepthStencilView>> _dsvMap;
		std::map<uint64_t, Ptr<ID3D11UnorderedAccessView>> _uavMap;
		int32_t _mappedSubResource;

		D3D11Texture() = default;
		const Ptr<ID3D11ShaderResourceView>	&  InitShaderResourceView(const D3D11_SHADER_RESOURCE_VIEW_DESC & desc);
		const Ptr<ID3D11RenderTargetView>		&  InitRenderTargetView(const D3D11_RENDER_TARGET_VIEW_DESC & desc);
		const Ptr<ID3D11DepthStencilView>		&  InitDepthStencilView(const D3D11_DEPTH_STENCIL_VIEW_DESC & desc);
		const Ptr<ID3D11UnorderedAccessView>	&  InitUnorderedAccessView(const D3D11_UNORDERED_ACCESS_VIEW_DESC & desc);
		void ExtractD3DBindFlags
			(bool hasInitData, uint32_t & d3dBindFlags, uint32_t & d3dCpuAccessFlags, D3D11_USAGE & d3dUsage, uint32_t & d3dMiscFlags);
	};
}

#endif