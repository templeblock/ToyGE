#include "ToyGE\D3D11\D3D11Texture.h"
#include "ToyGE\Kernel\Util.h"
#include "boost\functional\hash.hpp"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	D3D11Texture::D3D11Texture(const TextureDesc & desc)
		: Texture(desc),
		_mappedSubResource(-1)
	{

	}

	D3D11Texture::D3D11Texture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList)
		: Texture(desc, initDataList),
		_mappedSubResource(-1)
	{

	}

	bool D3D11Texture::CopyTo(const Ptr<Texture> & dst,
		int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
		int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox)
	{
		if (dst->Desc().format != _desc.format || dst->Desc().type != _desc.type)
			return false;

		auto & dstSize = dst->GetMipSize(dstMipLevel);
		auto & srcSize = dst->GetMipSize(dstMipLevel);

		int3 copySize;
		if (srcBox)
		{
			copySize.x = srcBox->right - srcBox->left;
			copySize.y = srcBox->bottom - srcBox->top;
			copySize.z = srcBox->back - srcBox->front;
		}
		else
		{
			copySize.x = std::get<0>(srcSize);
			copySize.y = std::get<1>(srcSize);
			copySize.z = std::get<2>(srcSize);
		}

		if (   xOffset + copySize.x > std::get<0>(dstSize)
			|| yOffset + copySize.y > std::get<1>(dstSize)
			|| zOffset + copySize.z > std::get<2>(dstSize))
			return false;

		/*if ( dst->GetMipSize(dstMipLevel) != GetMipSize(srcMipLevel) )
			return false;*/

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		Ptr<D3D11Texture> d3dDst = std::static_pointer_cast<D3D11Texture>(dst);

		auto dstSubIndex = ::D3D11CalcSubresource(
			dstMipLevel,
			dstArrayIndex,
			dst->Desc().mipLevels);

		auto srcSubIndex = ::D3D11CalcSubresource(
			srcMipLevel,
			srcArrayIndex,
			_desc.mipLevels);

		if (srcBox)
		{
			D3D11_BOX d3dSrcBox;
			d3dSrcBox.left = srcBox->left;
			d3dSrcBox.right = srcBox->right;
			d3dSrcBox.top = srcBox->top;
			d3dSrcBox.bottom = srcBox->bottom;
			d3dSrcBox.front = srcBox->front;
			d3dSrcBox.back = srcBox->back;

			re->RawD3DDeviceContex()->CopySubresourceRegion(
				d3dDst->RawD3DTexture().get(),
				dstSubIndex,
				xOffset,
				yOffset,
				zOffset,
				RawD3DTexture().get(),
				srcSubIndex,
				&d3dSrcBox);
		}
		else
		{ 
			re->RawD3DDeviceContex()->CopySubresourceRegion(
				d3dDst->RawD3DTexture().get(),
				dstSubIndex,
				xOffset,
				yOffset,
				zOffset,
				RawD3DTexture().get(),
				srcSubIndex,
				nullptr);
		}

		return true;
	}

	RenderDataDesc D3D11Texture::Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		uint32_t subIndex = ::D3D11CalcSubresource(
			mipLevel,
			arrayIndex,
			_desc.mipLevels);
		auto rc = re->RawD3DDeviceContex();
		auto rs = RawD3DTexture().get();
		rc->Map(rs, subIndex, static_cast<D3D11_MAP>(mapFlag), 0, &mappedResource);

		RenderDataDesc ret;
		ret.pData = mappedResource.pData;
		ret.rowPitch = mappedResource.RowPitch;
		ret.slicePitch = mappedResource.DepthPitch;

		_mappedSubResource = static_cast<int32_t>(subIndex);

		return ret;
	}

	void D3D11Texture::UnMap()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		re->RawD3DDeviceContex()->Unmap(RawD3DTexture().get(), static_cast<uint32_t>(_mappedSubResource));
	}

	const Ptr<ID3D11ShaderResourceView>&
		D3D11Texture::AcquireRawD3DShaderResourceView
		(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		static Ptr<ID3D11ShaderResourceView> ret;
		return ret;
	}
	const Ptr<ID3D11ShaderResourceView>&
		D3D11Texture::AcquireRawD3DShaderResourceView_Cube
		(int32_t firstMipLevel, int32_t numMipLevels, int32_t firstFaceOffset, int32_t numCubes, RenderFormat formatHint)
	{
		static Ptr<ID3D11ShaderResourceView> ret;
		return ret;
	}

	const Ptr<ID3D11RenderTargetView>&
		D3D11Texture::AcquireRawD3DRenderTargetView
		(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		static Ptr<ID3D11RenderTargetView> ret;
		return ret;
	}

	//const Ptr<ID3D11RenderTargetView>&
	//	D3D11Texture::AcquireRawD3DRenderTargetView_Cube
	//	(int mipLevel, int cubeIndex, CubeFace face, int numfaces)
	//{
	//	static Ptr<ID3D11RenderTargetView> ret;
	//	return ret;
	//}

	const Ptr<ID3D11DepthStencilView>&
		D3D11Texture::AcquireRawD3DDepthStencilView
		(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		static Ptr<ID3D11DepthStencilView> ret;
		return ret;
	}
	//const Ptr<ID3D11DepthStencilView>&
	//	D3D11Texture::AcquireRawD3DDepthStencilView_Cube
	//	(int mipLevel, int cubeIndex, CubeFace face, int numfaces)
	//{
	//	static Ptr<ID3D11DepthStencilView> ret;
	//	return ret;
	//}

	const Ptr<ID3D11UnorderedAccessView>&
		D3D11Texture::AcquireRawD3DUnorderedAccessView
		(int32_t mipLevel, int32_t firstArray, int32_t arraySize, RenderFormat formatHint)
	{
		static Ptr<ID3D11UnorderedAccessView> ret;
		return ret;
	}
	//const Ptr<ID3D11UnorderedAccessView>&
	//	D3D11Texture::AcquireRawD3DUnorderedAccessView_Cube
	//	(int mipLevel, int cubeIndex, CubeFace face, int numfaces)
	//{
	//	static Ptr<ID3D11UnorderedAccessView> ret;
	//	return ret;
	//}

	bool D3D11Texture::GenerateMips()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>( Global::GetRenderEngine() );

		if (_desc.bindFlag & TEXTURE_BIND_GENERATE_MIPS)
		{
			auto &srv = AcquireRawD3DShaderResourceView(0, _desc.mipLevels, 0, _desc.arraySize, RENDER_FORMAT_UNDEFINED);
			re->RawD3DDeviceContex()->GenerateMips(srv.get());
			return true;
		}

		return false;
	}

	const Ptr<ID3D11ShaderResourceView>	& D3D11Texture::InitShaderResourceView(const D3D11_SHADER_RESOURCE_VIEW_DESC & desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_SHADER_RESOURCE_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _srvMap.find(hashValue);
		if (itr == _srvMap.end())
		{
			ID3D11ShaderResourceView *pSRV = nullptr;
			re->RawD3DDevice()->CreateShaderResourceView(RawD3DTexture().get(), &desc, &pSRV);
			return _srvMap.insert(std::make_pair(hashValue, MakeComShared(pSRV))).first->second;
		}
		else
			return itr->second;
	}

	const Ptr<ID3D11RenderTargetView> & D3D11Texture::InitRenderTargetView(const D3D11_RENDER_TARGET_VIEW_DESC & desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_RENDER_TARGET_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _rtvMap.find(hashValue);
		if (itr == _rtvMap.end())
		{
			ID3D11RenderTargetView *pRTV = nullptr;
			re->RawD3DDevice()->CreateRenderTargetView(RawD3DTexture().get(), &desc, &pRTV);
			auto rtvShared = MakeComShared(pRTV);
			return _rtvMap.insert(std::make_pair(hashValue, rtvShared)).first->second;
		}
		else
			return itr->second;
	}

	const Ptr<ID3D11DepthStencilView> & D3D11Texture::InitDepthStencilView(const D3D11_DEPTH_STENCIL_VIEW_DESC & desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_DEPTH_STENCIL_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _dsvMap.find(hashValue);
		if (itr == _dsvMap.end())
		{
			ID3D11DepthStencilView *pDSV = nullptr;
			re->RawD3DDevice()->CreateDepthStencilView(RawD3DTexture().get(), &desc, &pDSV);
			return _dsvMap.insert(std::make_pair(hashValue, MakeComShared(pDSV))).first->second;
		}
		else
			return itr->second;
	}

	const Ptr<ID3D11UnorderedAccessView> & D3D11Texture::InitUnorderedAccessView(const D3D11_UNORDERED_ACCESS_VIEW_DESC & desc)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_UNORDERED_ACCESS_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _uavMap.find(hashValue);
		if (itr == _uavMap.end())
		{
			ID3D11UnorderedAccessView *pUAV = nullptr;
			re->RawD3DDevice()->CreateUnorderedAccessView(RawD3DTexture().get(), &desc, &pUAV);
			return _uavMap.insert(std::make_pair(hashValue, MakeComShared(pUAV))).first->second;
		}
		else
			return itr->second;
	}

	void D3D11Texture::ExtractD3DBindFlags
		(bool hasInitData, uint32_t & d3dBindFlags, uint32_t & d3dCpuAccessFlags, D3D11_USAGE & d3dUsage, uint32_t & d3dMiscFlags)
	{
		d3dBindFlags = d3dCpuAccessFlags = d3dMiscFlags = 0;

		d3dUsage = D3D11_USAGE_DEFAULT;

		if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
		{
			if (hasInitData)
				d3dUsage = D3D11_USAGE_IMMUTABLE;
			else
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with no initdata");
		}

		if (_desc.bindFlag & TEXTURE_BIND_SHADER_RESOURCE)
		{
			d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
			//d3dUsage = D3D11_USAGE_DEFAULT;
		}

		if (_desc.bindFlag & TEXTURE_BIND_STREAM_OUTPUT)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_STREAM_OUTPUT");

			d3dBindFlags |= D3D11_BIND_STREAM_OUTPUT;
		}

		if (_desc.bindFlag & TEXTURE_BIND_RENDER_TARGET)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_RENDER_TARGET");

			d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		
		if (_desc.bindFlag & TEXTURE_BIND_DEPTH_STENCIL)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_DEPTH_STENCIL");

			d3dBindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}

		if (_desc.bindFlag & TEXTURE_BIND_UNORDERED_ACCESS)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_UNORDERED_ACCESS");

			d3dBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}


		if (_desc.bindFlag & TEXTURE_BIND_GENERATE_MIPS)
		{
			if (_desc.bindFlag & TEXTURE_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with TEXTURE_BIND_GENERATE_MIPS");

			d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
			d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
			d3dMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}


		if ((_desc.bindFlag & TEXTURE_BIND_IMMUTABLE) &&
			(_desc.cpuAccess & CPU_ACCESS_WRITE))
		{
			ToyGE_ASSERT_FAIL("TEXTURE_BIND_IMMUTABLE with CPU_ACCESS_WRITE");
		}


		if ((_desc.cpuAccess & CPU_ACCESS_READ) && (_desc.cpuAccess & CPU_ACCESS_WRITE))
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			d3dUsage = D3D11_USAGE_STAGING;
		}
		else if (_desc.cpuAccess & CPU_ACCESS_READ)
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_READ;
			d3dUsage = D3D11_USAGE_STAGING;
		}
		else if (_desc.cpuAccess & CPU_ACCESS_WRITE)
		{
			d3dCpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
			d3dUsage = D3D11_USAGE_DYNAMIC;
		}

		if (_desc.type == TEXTURE_CUBE)
		{
			d3dMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
	}
}