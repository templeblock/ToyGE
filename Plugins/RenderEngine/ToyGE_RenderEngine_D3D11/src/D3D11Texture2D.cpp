#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void D3D11Texture2D::Init(const std::vector<RenderDataDesc> & initDataList)
	{
		D3D11Texture::Init(initDataList);

		bool bWithData = initDataList.size() > 0;

		auto adjustArraySize = _desc.bCube ? _desc.arraySize * 6 : _desc.arraySize;

		// Init desc
		D3D11_TEXTURE2D_DESC d3dTexDesc;
		d3dTexDesc.Format = GetD3DFormat(_desc.format);
		d3dTexDesc.Width = _desc.width;
		d3dTexDesc.Height = _desc.height;
		d3dTexDesc.ArraySize = adjustArraySize;
		d3dTexDesc.MipLevels = _desc.mipLevels;
		d3dTexDesc.SampleDesc.Count = _desc.sampleCount;
		d3dTexDesc.SampleDesc.Quality = _desc.sampleQuality;
		GetD3DTextureCreateFlags(bWithData, d3dTexDesc.BindFlags, d3dTexDesc.CPUAccessFlags, d3dTexDesc.MiscFlags, d3dTexDesc.Usage);

		if (IsCompress(_desc.format))
		{
			if (d3dTexDesc.Width > 1)
				d3dTexDesc.Width = (d3dTexDesc.Width + 3) / 4 * 4;
			if (d3dTexDesc.Height > 1)
				d3dTexDesc.Height = (d3dTexDesc.Height + 3) / 4 * 4;
		}

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture2D *pTexture2D = nullptr;

		// Init with data
		if (initDataList.size() > 0)
		{
			D3D11_SUBRESOURCE_DATA *pInitDataDesc = nullptr;
			std::vector<D3D11_SUBRESOURCE_DATA> initDataDescList(adjustArraySize * _desc.mipLevels);
			int32_t dataIndex = 0;
			for (int32_t arrayIndex = 0; arrayIndex != adjustArraySize; ++arrayIndex)
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
			D3D11RenderEngine::d3d11Device->CreateTexture2D(&d3dTexDesc, pInitDataDesc, &pTexture2D);
		}
		else
			D3D11RenderEngine::d3d11Device->CreateTexture2D(&d3dTexDesc, nullptr, &pTexture2D);

		_hardwareTexture2D = MakeComShared(pTexture2D);
	}

	void D3D11Texture2D::InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource)
	{
		_hardwareTexture2D = std::static_pointer_cast<ID3D11Texture2D>(hardwareResource);

		D3D11_TEXTURE2D_DESC desc;
		_hardwareTexture2D->GetDesc(&desc);
		//_desc.type = TEXTURE_2D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = desc.Height;
		_desc.depth = 1;
		_desc.arraySize = desc.ArraySize;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = desc.SampleDesc.Count;
		_desc.sampleQuality = desc.SampleDesc.Quality;
		GetFlagsFromD3D(desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags, _desc.bindFlag, _desc.cpuAccess, _desc.bCube);

		InitMipsSize();
	}

	Ptr<TextureShaderResourceView> D3D11Texture2D::CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat)
	{
		// Init d3d11 srv desc
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		if (!bCube)
		{
			if (firstArray == 0 && numArrays == 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = firstMip;
				desc.Texture2D.MipLevels = numMips;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = firstMip;
				desc.Texture2DArray.MipLevels = numMips;
				desc.Texture2DArray.FirstArraySlice = firstArray;
				desc.Texture2DArray.ArraySize = numArrays;
			}
		}
		else
		{
			if (numArrays == 1)
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				desc.TextureCube.MostDetailedMip = firstMip;
				desc.TextureCube.MipLevels = numMips;
			}
			else
			{
				desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
				desc.TextureCubeArray.MostDetailedMip = firstMip;
				desc.TextureCubeArray.MipLevels = numMips;
				desc.TextureCubeArray.First2DArrayFace = firstArray;
				desc.TextureCubeArray.NumCubes = numArrays;
			}
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 srv
		ID3D11ShaderResourceView *pSRV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateShaderResourceView(_hardwareTexture2D.get(), &desc, &pSRV);

		auto resultSRV = std::make_shared<D3D11TextureShaderResourceView>();
		resultSRV->hardwareSRV = MakeComShared(pSRV);

		return resultSRV;
	}

	Ptr<TextureUnorderedAccessView> D3D11Texture2D::CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 uav desc
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.ArraySize = numArrays;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 uav
		ID3D11UnorderedAccessView *pUAV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateUnorderedAccessView(_hardwareTexture2D.get(), &desc, &pUAV);

		auto resultUAV = std::make_shared<D3D11TextureUnorderedAccessView>();
		resultUAV->hardwareUAV = MakeComShared(pUAV);

		return resultUAV;
	}

	Ptr<TextureRenderTargetView> D3D11Texture2D::CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 rtv desc
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.ArraySize = numArrays;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11RenderTargetView *pRTV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateRenderTargetView(_hardwareTexture2D.get(), &desc, &pRTV);

		auto resultRTV = std::make_shared<D3D11TextureRenderTargetView>();
		resultRTV->hardwareRTV = MakeComShared(pRTV);

		return resultRTV;
	}

	Ptr<TextureDepthStencilView> D3D11Texture2D::CreateDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 dsv desc
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = firstArray;
			desc.Texture2DArray.ArraySize = numArrays;
			desc.Texture2DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);
		desc.Flags = 0;

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11DepthStencilView *pDSV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateDepthStencilView(_hardwareTexture2D.get(), &desc, &pDSV);

		auto resultDSV = std::make_shared<D3D11TextureDepthStencilView>();
		resultDSV->hardwareDSV = MakeComShared(pDSV);

		return resultDSV;
	}
}