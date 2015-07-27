#include "ToyGE\D3D11\D3D11Texture1D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"

namespace ToyGE
{
	D3D11Texture1D::D3D11Texture1D(const TextureDesc & desc)
		: D3D11Texture1D(desc, std::vector<RenderDataDesc>())
	{

	}

	D3D11Texture1D::D3D11Texture1D(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
		: D3D11Texture(desc, initDataList)
	{
		D3D11_TEXTURE1D_DESC d3dTexDesc;
		CreateRawD3DTexture1D_Desc(initDataList.size() > 0, d3dTexDesc);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture1D *pTexture1D = nullptr;

		if (initDataList.size() > 0)
		{
			D3D11_SUBRESOURCE_DATA *pInitDataDesc = nullptr;
			std::vector<D3D11_SUBRESOURCE_DATA> initDataDescList(_desc.arraySize * _desc.mipLevels);
			int32_t dataIndex = 0;
			for (int32_t arrayIndex = 0; arrayIndex != _desc.arraySize; ++arrayIndex)
			{
				for (int32_t mipIndex = 0; mipIndex != _desc.mipLevels; ++mipIndex)
				{
					auto &initData = initDataList[dataIndex];
					initDataDescList[dataIndex].pSysMem = initData.pData;
					initDataDescList[dataIndex].SysMemPitch = static_cast<uint32_t>(initData.rowPitch);
					initDataDescList[dataIndex].SysMemSlicePitch = static_cast<uint32_t>(initData.slicePitch);

					++dataIndex;
				}
			}
			pInitDataDesc = &initDataDescList[0];
			re->RawD3DDevice()->CreateTexture1D(&d3dTexDesc, pInitDataDesc, &pTexture1D);
		}
		else
			re->RawD3DDevice()->CreateTexture1D(&d3dTexDesc, nullptr, &pTexture1D);
		
		_rawD3DTexture1D = MakeComShared(pTexture1D);
	}

	std::shared_ptr<D3D11Texture1D>
		D3D11Texture1D::CreateFromRawD3D(
		const Ptr<ID3D11Device> & rawDevice,
		const Ptr<ID3D11Texture1D> & rawTexture1D)
	{
		auto texture = std::shared_ptr<D3D11Texture1D>(new D3D11Texture1D());
		texture->_rawD3DTexture1D = rawTexture1D;
		texture->InitFromRawD3DTexture();
		return texture;
	}

	const Ptr<ID3D11ShaderResourceView>&
		D3D11Texture1D::AcquireRawD3DShaderResourceView(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MostDetailedMip = firstMipLevel;
			desc.Texture1D.MipLevels = numMipLevels;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MostDetailedMip = firstMipLevel;
			desc.Texture1DArray.MipLevels = numMipLevels;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = arraySize;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitShaderResourceView(desc);
	}

	const Ptr<ID3D11RenderTargetView>&
		D3D11Texture1D::AcquireRawD3DRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.ArraySize = arraySize;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitRenderTargetView(desc);
	}

	const Ptr<ID3D11DepthStencilView>&
		D3D11Texture1D::AcquireRawD3DDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = arraySize;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);
		desc.Flags = 0;

		return InitDepthStencilView(desc);
	}

	const Ptr<ID3D11UnorderedAccessView>&
		D3D11Texture1D::AcquireRawD3DUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = arraySize;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitUnorderedAccessView(desc);
	}


	void D3D11Texture1D::InitFromRawD3DTexture()
	{
		D3D11_TEXTURE1D_DESC desc;
		_rawD3DTexture1D->GetDesc(&desc);
		_desc.type = TEXTURE_1D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = 0;
		_desc.depth = 0;
		_desc.arraySize = desc.ArraySize;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = 1;
		_desc.sampleQuality = 0;

		_desc.mipLevels = ComputeMipLevels(_desc.mipLevels, _desc.width, _desc.height, _desc.depth, _mipSizeMap);
	}

	void D3D11Texture1D::CreateRawD3DTexture1D_Desc(bool hasInitData, D3D11_TEXTURE1D_DESC & texture1D_Desc)
	{
		texture1D_Desc.Format = GetD3DFormat(_desc.format);
		texture1D_Desc.Width = _desc.width;
		texture1D_Desc.ArraySize = _desc.arraySize;
		texture1D_Desc.MipLevels = _desc.mipLevels;

		ExtractD3DBindFlags(hasInitData, texture1D_Desc.BindFlags, texture1D_Desc.CPUAccessFlags, texture1D_Desc.Usage, texture1D_Desc.MiscFlags);
	}
}