#include "ToyGE\D3D11\D3D11Texture1D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void D3D11Texture1D::Init(const std::vector<RenderDataDesc> & initDataList)
	{
		D3D11Texture::Init(initDataList);

		bool bWithData = initDataList.size() > 0;

		// Init desc
		D3D11_TEXTURE1D_DESC d3dTexDesc;
		d3dTexDesc.Format = GetD3DFormat(_desc.format);
		d3dTexDesc.Width = _desc.width;
		d3dTexDesc.ArraySize = _desc.arraySize;
		d3dTexDesc.MipLevels = _desc.mipLevels;
		GetD3DTextureCreateFlags(bWithData, d3dTexDesc.BindFlags, d3dTexDesc.CPUAccessFlags, d3dTexDesc.MiscFlags, d3dTexDesc.Usage);

		if (IsCompress(_desc.format) && d3dTexDesc.Width > 1)
			d3dTexDesc.Width = (d3dTexDesc.Width + 3) / 4 * 4;

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture1D *pTexture1D = nullptr;

		// Init with data
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
			D3D11RenderEngine::d3d11Device->CreateTexture1D(&d3dTexDesc, pInitDataDesc, &pTexture1D);
		}
		else
			D3D11RenderEngine::d3d11Device->CreateTexture1D(&d3dTexDesc, nullptr, &pTexture1D);

		_hardwareTexture1D = MakeComShared(pTexture1D);
	}

	void D3D11Texture1D::InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource)
	{
		_hardwareTexture1D = std::static_pointer_cast<ID3D11Texture1D>(hardwareResource);
		
		D3D11_TEXTURE1D_DESC desc;
		_hardwareTexture1D->GetDesc(&desc);
		//_desc.type = TEXTURE_1D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = 1;
		_desc.depth = 1;
		_desc.arraySize = desc.ArraySize;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = 1;
		_desc.sampleQuality = 0;
		GetFlagsFromD3D(desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags, _desc.bindFlag, _desc.cpuAccess, _desc.bCube);

		InitMipsSize();
	}

	Ptr<TextureShaderResourceView> D3D11Texture1D::CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat)
	{
		// Init d3d11 srv desc
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MostDetailedMip = firstMip;
			desc.Texture1D.MipLevels = numMips;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MostDetailedMip = firstMip;
			desc.Texture1DArray.MipLevels = numMips;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = numArrays;
		}

		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 srv
		ID3D11ShaderResourceView *pSRV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateShaderResourceView(_hardwareTexture1D.get(), &desc, &pSRV);

		auto resultSRV = std::make_shared<D3D11TextureShaderResourceView>();
		resultSRV->hardwareSRV = MakeComShared(pSRV);

		return resultSRV;
	}

	Ptr<TextureUnorderedAccessView> D3D11Texture1D::CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 uav desc
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = numArrays;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 uav
		ID3D11UnorderedAccessView *pUAV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateUnorderedAccessView(_hardwareTexture1D.get(), &desc, &pUAV);

		auto resultUAV = std::make_shared<D3D11TextureUnorderedAccessView>();
		resultUAV->hardwareUAV = MakeComShared(pUAV);

		return resultUAV;
	}

	Ptr<TextureRenderTargetView> D3D11Texture1D::CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 rtv desc
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.ArraySize = numArrays;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11RenderTargetView *pRTV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateRenderTargetView(_hardwareTexture1D.get(), &desc, &pRTV);

		auto resultRTV = std::make_shared<D3D11TextureRenderTargetView>();
		resultRTV->hardwareRTV = MakeComShared(pRTV);

		return resultRTV;
	}

	Ptr<TextureDepthStencilView> D3D11Texture1D::CreateDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 dsv desc
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (firstArray == 0 && numArrays == 1 && _desc.arraySize == 1)
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = mipLevel;
		}
		else
		{
			desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.FirstArraySlice = firstArray;
			desc.Texture1DArray.ArraySize = numArrays;
			desc.Texture1DArray.MipSlice = mipLevel;
		}
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);
		desc.Flags = 0;

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11DepthStencilView *pDSV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateDepthStencilView(_hardwareTexture1D.get(), &desc, &pDSV);

		auto resultDSV = std::make_shared<D3D11TextureDepthStencilView>();
		resultDSV->hardwareDSV = MakeComShared(pDSV);

		return resultDSV;
	}
}