#include "ToyGE\D3D11\D3D11Texture3D.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	void D3D11Texture3D::Init(const std::vector<RenderDataDesc> & initDataList)
	{
		D3D11Texture::Init(initDataList);

		bool bWithData = initDataList.size() > 0;

		// Init desc
		D3D11_TEXTURE3D_DESC d3dTexDesc;
		d3dTexDesc.Format = GetD3DFormat(_desc.format);
		d3dTexDesc.Width = _desc.width;
		d3dTexDesc.Height = _desc.height;
		d3dTexDesc.Depth = _desc.depth;
		d3dTexDesc.MipLevels = _desc.mipLevels;
		GetD3DTextureCreateFlags(bWithData, d3dTexDesc.BindFlags, d3dTexDesc.CPUAccessFlags, d3dTexDesc.MiscFlags, d3dTexDesc.Usage);

		if (IsCompress(_desc.format))
		{
			if (d3dTexDesc.Width > 1)
				d3dTexDesc.Width = (d3dTexDesc.Width + 3) / 4 * 4;
			if (d3dTexDesc.Height > 1)
				d3dTexDesc.Height = (d3dTexDesc.Height + 3) / 4 * 4;
		}

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Texture3D *pTexture3D = nullptr;

		// Init with data
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
			D3D11RenderEngine::d3d11Device->CreateTexture3D(&d3dTexDesc, pInitDataDesc, &pTexture3D);
		}
		else
			D3D11RenderEngine::d3d11Device->CreateTexture3D(&d3dTexDesc, nullptr, &pTexture3D);

		_hardwareTexture3D = MakeComShared(pTexture3D);
	}

	void D3D11Texture3D::InitFromHardware(const Ptr<ID3D11Resource> & hardwareResource)
	{
		_hardwareTexture3D = std::static_pointer_cast<ID3D11Texture3D>(hardwareResource);

		D3D11_TEXTURE3D_DESC desc;
		memset(&desc, 0, sizeof(desc));
		_hardwareTexture3D->GetDesc(&desc);
		//_desc.type = TEXTURE_3D;
		_desc.format = GetRenderFormat(desc.Format);
		_desc.width = desc.Width;
		_desc.height = desc.Height;
		_desc.depth = desc.Depth;
		_desc.arraySize = 1;
		_desc.mipLevels = desc.MipLevels;
		_desc.sampleCount = 1;
		_desc.sampleQuality = 0;
		GetFlagsFromD3D(desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags, _desc.bindFlag, _desc.cpuAccess, _desc.bCube);

		InitMipsSize();
	}

	Ptr<TextureShaderResourceView> D3D11Texture3D::CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat)
	{
		// Init d3d11 srv desc
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MostDetailedMip = firstMip;
		desc.Texture3D.MipLevels = numMips;
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 srv
		ID3D11ShaderResourceView *pSRV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateShaderResourceView(_hardwareTexture3D.get(), &desc, &pSRV);

		auto resultSRV = std::make_shared<D3D11TextureShaderResourceView>();
		resultSRV->hardwareSRV = MakeComShared(pSRV);

		return resultSRV;
	}

	Ptr<TextureUnorderedAccessView> D3D11Texture3D::CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 uav desc
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = mipLevel;
		desc.Texture3D.FirstWSlice = firstArray;
		desc.Texture3D.WSize = numArrays;
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 uav
		ID3D11UnorderedAccessView *pUAV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateUnorderedAccessView(_hardwareTexture3D.get(), &desc, &pUAV);

		auto resultUAV = std::make_shared<D3D11TextureUnorderedAccessView>();
		resultUAV->hardwareUAV = MakeComShared(pUAV);

		return resultUAV;
	}

	Ptr<TextureRenderTargetView> D3D11Texture3D::CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat)
	{
		// Init d3d11 rtv desc
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		desc.Texture3D.MipSlice = mipLevel;
		desc.Texture3D.FirstWSlice = firstArray;
		desc.Texture3D.WSize = numArrays;
		if (viewFormat == RENDER_FORMAT_UNDEFINED)
			desc.Format = GetD3DFormat(_desc.format);
		else
			desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11RenderTargetView *pRTV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateRenderTargetView(_hardwareTexture3D.get(), &desc, &pRTV);

		auto resultRTV = std::make_shared<D3D11TextureRenderTargetView>();
		resultRTV->hardwareRTV = MakeComShared(pRTV);

		return resultRTV;
	}
}