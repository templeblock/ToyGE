#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"
#include "boost\functional\hash.hpp"
#include "boost\enable_shared_from_this.hpp"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	D3D11RenderBuffer::D3D11RenderBuffer(const RenderBufferDesc & desc, const void *pInitData)
		: RenderBuffer(desc, pInitData)
	{
		Init(desc, pInitData);
	}

	//D3D11RenderBuffer::D3D11RenderBuffer(const RenderBufferDesc & desc, const void *pInitData,
	//	VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc)
	//	: RenderBuffer(desc, pInitData, vertexBufferType, vertexElementDesc)
	//{
	//	Init(desc, pInitData);
	//}

	RenderDataDesc D3D11RenderBuffer::Map(MapType mapFlag)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>( Global::GetRenderEngine() );
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		re->RawD3DDeviceContex()->Map(RawD3DBuffer().get(), 0, static_cast<D3D11_MAP>(mapFlag), 0, &mappedResource);
		RenderDataDesc ret;
		ret.pData = mappedResource.pData;
		ret.rowPitch = mappedResource.RowPitch;
		ret.slicePitch = mappedResource.DepthPitch;
		return ret;
	}

	void D3D11RenderBuffer::UnMap()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		re->RawD3DDeviceContex()->Unmap(RawD3DBuffer().get(), 0);
	}

	void D3D11RenderBuffer::CopyStructureCountTo(
		const Ptr<RenderBuffer> & dst,
		uint32_t bytesOffset,
		int32_t firstElement,
		int32_t numElements,
		RenderFormat format,
		uint32_t uavFlag)
	{
		auto uaVView = AcquireRawD3DUnorderedAccessView(firstElement, numElements, format, uavFlag);
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		auto d3dContex = re->RawD3DDeviceContex().get();

		auto d3dDstBuffer = std::static_pointer_cast<D3D11RenderBuffer>(dst);

		d3dContex->CopyStructureCount(d3dDstBuffer->RawD3DBuffer().get(), bytesOffset, uaVView.get());
	}

	bool D3D11RenderBuffer::CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const
	{
		if (!dst)
			return false;

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		auto dstD3DBuffer = std::static_pointer_cast<D3D11RenderBuffer>(dst);

		D3D11_BOX srcBox;
		srcBox.left = srcBytesOffset;
		srcBox.right = srcBox.left + cpyBytesSize;
		srcBox.front = 0;
		srcBox.back = 1;
		srcBox.top = 0;
		srcBox.bottom = 1;

		re->RawD3DDeviceContex()->CopySubresourceRegion(dstD3DBuffer->RawD3DBuffer().get(), 0, dstBytesOffset, 0, 0, RawD3DBuffer().get(), 0, &srcBox);

		return true;
	}

	const Ptr<ID3D11ShaderResourceView>&
		D3D11RenderBuffer::AcquireRawD3DShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat format)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (_desc.bindFlag & BUFFER_BIND_STRUCTURED)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			desc.BufferEx.FirstElement = firstElement;
			desc.BufferEx.NumElements = numElements;
			desc.BufferEx.Flags = 0;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}
		desc.Format = GetD3DFormat(format);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_SHADER_RESOURCE_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _srvMap.find(hashValue);
		if (itr == _srvMap.end())
		{
			ID3D11ShaderResourceView *pSRV = nullptr;
			re->RawD3DDevice()->CreateShaderResourceView(RawD3DBuffer().get(), &desc, &pSRV);
			return _srvMap.insert(std::make_pair(hashValue, MakeComShared(pSRV))).first->second;
		}
		else
			return itr->second;
	}

	const Ptr<ID3D11RenderTargetView>&
		D3D11RenderBuffer::AcquireRawD3DRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat format)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
		desc.Format = GetD3DFormat(format);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_RENDER_TARGET_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _rtvMap.find(hashValue);
		if (itr == _rtvMap.end())
		{
			ID3D11RenderTargetView *pRTV = nullptr;
			re->RawD3DDevice()->CreateRenderTargetView(RawD3DBuffer().get(), &desc, &pRTV);
			return _rtvMap.insert(std::make_pair(hashValue, MakeComShared(pRTV))).first->second;
		}
		else
			return itr->second;
	}

	const Ptr<ID3D11UnorderedAccessView>&
		D3D11RenderBuffer::AcquireRawD3DUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat format, uint32_t uavFlags)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;

		desc.Buffer.Flags = 0;
		if (uavFlags & BUFFER_UAV_RAW)
			desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
		if (uavFlags & BUFFER_UAV_APPEND)
			desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_APPEND;
		if (uavFlags & BUFFER_UAV_COUNTER)
			desc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_COUNTER;

		desc.Format = GetD3DFormat(format);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		const char *pData = reinterpret_cast<const char*>(const_cast<D3D11_UNORDERED_ACCESS_VIEW_DESC*>(&desc));
		uint64_t hashValue = boost::hash_range(pData, pData + sizeof(desc));
		auto itr = _uavMap.find(hashValue);
		if (itr == _uavMap.end())
		{
			ID3D11UnorderedAccessView *pUAV = nullptr;
			re->RawD3DDevice()->CreateUnorderedAccessView(RawD3DBuffer().get(), &desc, &pUAV);
			return _uavMap.insert(std::make_pair(hashValue, MakeComShared(pUAV))).first->second;
		}
		else
			return itr->second;
	}

	void D3D11RenderBuffer::Init(const RenderBufferDesc & createDesc, const void *pInitData)
	{
		D3D11_BUFFER_DESC d3dBufferDesc;
		CreateBufferDesc(nullptr != pInitData, d3dBufferDesc);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Buffer *pBuffer = nullptr;


		if (nullptr != pInitData)
		{
			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = pInitData;
			re->RawD3DDevice()->CreateBuffer(&d3dBufferDesc, &data, &pBuffer);
		}
		else
			re->RawD3DDevice()->CreateBuffer(&d3dBufferDesc, nullptr, &pBuffer);

		_rawD3DBuffer = MakeComShared(pBuffer);
	}

	void D3D11RenderBuffer::CreateBufferDesc(bool hasInitData, D3D11_BUFFER_DESC & outDesc)
	{
		outDesc.ByteWidth = static_cast<uint32_t>(_desc.elementSize) * static_cast<uint32_t>(_desc.numElements);
		outDesc.StructureByteStride = static_cast<uint32_t>(_desc.structedByteStride);

		ExtractD3DBindFlags(hasInitData, outDesc.BindFlags, outDesc.CPUAccessFlags, outDesc.Usage, outDesc.MiscFlags);
	}

	void D3D11RenderBuffer::ExtractD3DBindFlags(
		bool hasInitData,
		uint32_t & d3dBindFlags,
		uint32_t & d3dCpuAccessFlags,
		D3D11_USAGE & d3dUsage,
		uint32_t & d3dMiscFlags)
	{
		d3dBindFlags = d3dCpuAccessFlags = d3dMiscFlags = 0;
		d3dUsage = D3D11_USAGE_DEFAULT;

		if (_desc.bindFlag & BUFFER_BIND_IMMUTABLE)
		{
			if (hasInitData == false)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with no initdata");

			d3dUsage = D3D11_USAGE_IMMUTABLE;
		}

		if (_desc.bindFlag & BUFFER_BIND_CONSTANT)
		{
			d3dBindFlags |= D3D11_BIND_CONSTANT_BUFFER;
		}

		if (_desc.bindFlag & BUFFER_BIND_VERTEX)
		{
			d3dBindFlags |= D3D11_BIND_VERTEX_BUFFER;
		}

		if (_desc.bindFlag & BUFFER_BIND_INDEX)
		{
			d3dBindFlags |= D3D11_BIND_INDEX_BUFFER;
		}

		if (_desc.bindFlag & BUFFER_BIND_UNORDERED_ACCESS)
		{
			if (_desc.bindFlag & BUFFER_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with BUFFER_BIND_UNORDERED_ACCESS");

			d3dBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		if (_desc.bindFlag & BUFFER_BIND_SHADER_RESOURCE)
		{
			d3dBindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		if (_desc.bindFlag & BUFFER_BIND_RENDER_TARGET)
		{
			if (_desc.bindFlag & BUFFER_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with BUFFER_BIND_RENDER_TARGET");
			
			d3dBindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		if (_desc.bindFlag & BUFFER_BIND_STRUCTURED)
		{
			d3dMiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}

		if (_desc.bindFlag & BUFFER_BIND_INDIRECT_ARGS)
		{
			d3dMiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}

		if ( (_desc.bindFlag & BUFFER_BIND_IMMUTABLE) 
			&& (_desc.cpuAccess & CPU_ACCESS_WRITE) )
		{
			ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with CPU_ACCESS_WRITE");
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

	}
}