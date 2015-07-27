#include "ToyGE\D3D11\D3D11Texture3D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"

namespace ToyGE
{
	D3D11Texture3D::D3D11Texture3D(const TextureDesc & desc)
		: D3D11Texture3D(desc, std::vector<RenderDataDesc>())
	{

	}

	D3D11Texture3D::D3D11Texture3D(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
		: D3D11Texture(desc, initDataList)
	{
		D3D11_TEXTURE3D_DESC d3dTexDesc;
		CreateRawD3DTexture3D_Desc(initDataList.size() > 0, d3dTexDesc);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture3D *pTexture3D = nullptr;

		if (initDataList.size() > 0)
		{
			D3D11_SUBRESOURCE_DATA *pInitDataDesc = nullptr;
			std::vector<D3D11_SUBRESOURCE_DATA> initDataDescList(_desc.arraySize * _desc.mipLevels);
			int dataIndex = 0;
			for (int arrayIndex = 0; arrayIndex != _desc.arraySize; ++arrayIndex)
			{
				for (int mipIndex = 0; mipIndex != _desc.mipLevels; ++mipIndex)
				{
					auto &initData = initDataList[dataIndex];
					initDataDescList[dataIndex].pSysMem = initData.pData;
					initDataDescList[dataIndex].SysMemPitch = static_cast<uint32_t>(initData.rowPitch);
					initDataDescList[dataIndex].SysMemSlicePitch = static_cast<uint32_t>(initData.slicePitch);

					++dataIndex;
				}
			}
			pInitDataDesc = &initDataDescList[0];
			re->RawD3DDevice()->CreateTexture3D(&d3dTexDesc, pInitDataDesc, &pTexture3D);
		}
		else
			re->RawD3DDevice()->CreateTexture3D(&d3dTexDesc, nullptr, &pTexture3D);

		_rawD3DTexture3D = MakeComShared(pTexture3D);
	}

	Ptr<D3D11Texture3D>
		D3D11Texture3D::CreateFromRawD3D(
		const Ptr<ID3D11Device> & rawDevice,
		const Ptr<ID3D11Texture3D> & rawTexture3D)
	{
		auto texture = Ptr<D3D11Texture3D>(new D3D11Texture3D());
		texture->_rawD3DTexture3D = rawTexture3D;
		texture->InitFromRawD3DTexture();
		return texture;
	}

	const Ptr<ID3D11ShaderResourceView>&
		D3D11Texture3D::AcquireRawD3DShaderResourceView(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MostDetailedMip = firstMipLevel;
		desc.Texture3D.MipLevels = numMipLevels;
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitShaderResourceView(desc);
	}

	const Ptr<ID3D11RenderTargetView>&
		D3D11Texture3D::AcquireRawD3DRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = mipLevel;
		desc.Texture3D.FirstWSlice = firstArray;
		desc.Texture3D.WSize = arraySize;
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitRenderTargetView(desc);
	}

	const Ptr<ID3D11UnorderedAccessView>&
		D3D11Texture3D::AcquireRawD3DUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = mipLevel;
		desc.Texture3D.FirstWSlice = firstArray;
		desc.Texture3D.WSize = arraySize;
		if (formatHint == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(formatHint);

		return InitUnorderedAccessView(desc);
	}


	void D3D11Texture3D::InitFromRawD3DTexture()
	{
		D3D11_TEXTURE3D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		_rawD3DTexture3D->GetDesc(&desc);
		_desc.type = TEXTURE_3D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = 0;
		_desc.depth = 0;
		_desc.arraySize = 1;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = 1;
		_desc.sampleQuality = 0;

		_desc.mipLevels = ComputeMipLevels(_desc.mipLevels, _desc.width, _desc.height, _desc.depth, _mipSizeMap);
	}

	void D3D11Texture3D::CreateRawD3DTexture3D_Desc(bool hasInitData, D3D11_TEXTURE3D_DESC & texture3D_Desc)
	{
		texture3D_Desc.Format = GetD3DFormat(_desc.format);
		texture3D_Desc.Width = _desc.width;
		texture3D_Desc.Height = _desc.height;
		texture3D_Desc.Depth = _desc.depth;
		texture3D_Desc.MipLevels = _desc.mipLevels;

		ExtractD3DBindFlags(hasInitData, texture3D_Desc.BindFlags, texture3D_Desc.CPUAccessFlags, texture3D_Desc.Usage, texture3D_Desc.MiscFlags);
	}
}