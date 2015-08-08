#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	D3D11Texture2D::D3D11Texture2D(const TextureDesc & desc)
		: D3D11Texture2D(desc, std::vector<RenderDataDesc>())
	{

	}

	D3D11Texture2D::D3D11Texture2D(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
		: D3D11Texture(desc, initDataList)
	{
		D3D11_TEXTURE2D_DESC d3dTexDesc;
		CreateRawD3DTexture2D_Desc(initDataList.size() > 0, d3dTexDesc);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture2D *pTexture2D = nullptr;

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
			re->RawD3DDevice()->CreateTexture2D(&d3dTexDesc, pInitDataDesc, &pTexture2D);
		}
		else
			re->RawD3DDevice()->CreateTexture2D(&d3dTexDesc, nullptr, &pTexture2D);
	

//#if defined(DEBUG) || defined(_DEBUG)
//		if (!pTexture2D)
//		{
//			_asm int 3;
//		}
//#endif
		_rawD3DTexture2D = MakeComShared(pTexture2D);
	}

	Ptr<D3D11Texture2D> 
		D3D11Texture2D::CreateFromRawD3D(
		const Ptr<ID3D11Device> & rawDevice,
		const Ptr<ID3D11Texture2D> & rawTexture2D)
	{
		auto texture = Ptr<D3D11Texture2D>(new D3D11Texture2D());
		texture->_rawD3DTexture2D = rawTexture2D;
		texture->InitFromRawD3DTexture();
		return texture;
	}

	const Ptr<ID3D11ShaderResourceView>& 
		D3D11Texture2D::AcquireRawD3DShaderResourceView(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.type == TEXTURE_2D && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MostDetailedMip = firstMipLevel;
			desc.Texture2D.MipLevels = numMipLevels;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MostDetailedMip = firstMipLevel;
			desc.Texture2DArray.MipLevels = numMipLevels;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.ArraySize = arraySize;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitShaderResourceView(desc);
	}

	const Ptr<ID3D11RenderTargetView>&
		D3D11Texture2D::AcquireRawD3DRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.type == TEXTURE_2D && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitRenderTargetView(desc);
	}

	const Ptr<ID3D11DepthStencilView>&
		D3D11Texture2D::AcquireRawD3DDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.type == TEXTURE_2D && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);
		desc.Flags = 0;

		return InitDepthStencilView(desc);
	}

	const Ptr<ID3D11UnorderedAccessView>&
		D3D11Texture2D::AcquireRawD3DUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (firstArray == 0 && arraySize == 1 && _desc.type == TEXTURE_2D && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitUnorderedAccessView(desc);
	}


	void D3D11Texture2D::InitFromRawD3DTexture()
	{
		D3D11_TEXTURE2D_DESC desc;
		_rawD3DTexture2D->GetDesc(&desc);
		_desc.type = TEXTURE_2D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = desc.Height;
		_desc.depth = 0;
		_desc.arraySize = desc.ArraySize;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = desc.SampleDesc.Count;
		_desc.sampleQuality = desc.SampleDesc.Quality;

		_desc.mipLevels = ComputeMipLevels(_desc.mipLevels, _desc.width, _desc.height, _desc.depth, _mipSizeMap);
	}

	void D3D11Texture2D::CreateRawD3DTexture2D_Desc(bool hasInitData, D3D11_TEXTURE2D_DESC & texture2D_Desc)
	{
		texture2D_Desc.Format = GetD3DFormat(_desc.format);
		texture2D_Desc.Width = _desc.width;
		texture2D_Desc.Height = _desc.height;
		texture2D_Desc.ArraySize = _desc.arraySize;
		texture2D_Desc.MipLevels = _desc.mipLevels;
		texture2D_Desc.SampleDesc.Count = _desc.sampleCount;
		texture2D_Desc.SampleDesc.Quality = _desc.sampleQuality;

		if (IsCompress(_desc.format))
		{
			if(texture2D_Desc.Width > 1)
				texture2D_Desc.Width = (texture2D_Desc.Width + 3) / 4 * 4;
			if (texture2D_Desc.Height > 1)
				texture2D_Desc.Height = (texture2D_Desc.Height + 3) / 4 * 4;
		}

		ExtractD3DBindFlags(hasInitData, texture2D_Desc.BindFlags, texture2D_Desc.CPUAccessFlags, texture2D_Desc.Usage, texture2D_Desc.MiscFlags);
	}
}