#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	static void CreateBufferDesc(const RenderBufferDesc & bufferDesc, bool bWithInitData, D3D11_BUFFER_DESC & outD3DBufferDesc)
	{
		outD3DBufferDesc.ByteWidth = static_cast<uint32_t>(bufferDesc.elementSize * bufferDesc.numElements);
		outD3DBufferDesc.StructureByteStride = bufferDesc.bStructured ? static_cast<uint32_t>(bufferDesc.elementSize) : 0;

		outD3DBufferDesc.BindFlags = outD3DBufferDesc.CPUAccessFlags = outD3DBufferDesc.MiscFlags = 0;
		outD3DBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if (bufferDesc.bindFlag & BUFFER_BIND_IMMUTABLE)
		{
			if (bWithInitData == false)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with no initdata");

			outD3DBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_CONSTANT)
		{
			outD3DBufferDesc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_VERTEX)
		{
			outD3DBufferDesc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_INDEX)
		{
			outD3DBufferDesc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_UNORDERED_ACCESS)
		{
			if (bufferDesc.bindFlag & BUFFER_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with BUFFER_BIND_UNORDERED_ACCESS");

			outD3DBufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_SHADER_RESOURCE)
		{
			outD3DBufferDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_RENDER_TARGET)
		{
			if (bufferDesc.bindFlag & BUFFER_BIND_IMMUTABLE)
				ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with BUFFER_BIND_RENDER_TARGET");

			outD3DBufferDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		if (bufferDesc.bStructured)
		{
			outD3DBufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_INDIRECT_ARGS)
		{
			outD3DBufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}

		if (bufferDesc.bindFlag & BUFFER_BIND_RAW)
		{
			outD3DBufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		}

		if ((bufferDesc.bindFlag & BUFFER_BIND_IMMUTABLE)
			&& (bufferDesc.cpuAccess & CPU_ACCESS_WRITE))
		{
			ToyGE_ASSERT_FAIL("BUFFER_BIND_IMMUTABLE with CPU_ACCESS_WRITE");
		}

		if ((bufferDesc.cpuAccess & CPU_ACCESS_READ) && (bufferDesc.cpuAccess & CPU_ACCESS_WRITE))
		{
			outD3DBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			outD3DBufferDesc.Usage = D3D11_USAGE_STAGING;
		}
		else if (bufferDesc.cpuAccess & CPU_ACCESS_READ)
		{
			outD3DBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			outD3DBufferDesc.Usage = D3D11_USAGE_STAGING;
		}
		else if (bufferDesc.cpuAccess & CPU_ACCESS_WRITE)
		{
			outD3DBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			outD3DBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		}
	}

	void D3D11RenderBuffer::Init(const void * pInitData, bool bCacheData)
	{
		RenderBuffer::Init(pInitData, bCacheData);

		D3D11_BUFFER_DESC d3dBufferDesc;
		memset(&d3dBufferDesc, 0, sizeof(d3dBufferDesc));
		ToyGE::CreateBufferDesc(_desc, nullptr != pInitData, d3dBufferDesc);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11Buffer *pBuffer = nullptr;

		if (pInitData != nullptr)
		{
			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = pInitData;
			D3D11RenderEngine::d3d11Device->CreateBuffer(&d3dBufferDesc, &data, &pBuffer);
		}
		else
			D3D11RenderEngine::d3d11Device->CreateBuffer(&d3dBufferDesc, nullptr, &pBuffer);

		_hardwareBuffer = MakeComShared(pBuffer);
	}

	RenderDataDesc D3D11RenderBuffer::Map(MapType mapFlag)
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>( Global::GetRenderEngine() );
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		D3D11RenderEngine::d3d11DeviceContext->Map(_hardwareBuffer.get(), 0, static_cast<D3D11_MAP>(mapFlag), 0, &mappedResource);
		RenderDataDesc ret;
		ret.pData = mappedResource.pData;
		ret.rowPitch = mappedResource.RowPitch;
		ret.slicePitch = mappedResource.DepthPitch;

		if (mapFlag != MAP_READ)
			_bDirty = true;

		return ret;
	}

	void D3D11RenderBuffer::UnMap()
	{
		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		D3D11RenderEngine::d3d11DeviceContext->Unmap(_hardwareBuffer.get(), 0);
	}

	bool D3D11RenderBuffer::CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const
	{
		if (!dst)
			return false;

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		auto dstD3DBuffer = std::dynamic_pointer_cast<D3D11RenderBuffer>(dst);

		D3D11_BOX srcBox;
		srcBox.left = srcBytesOffset;
		srcBox.right = srcBox.left + cpyBytesSize;
		srcBox.front = 0;
		srcBox.back = 1;
		srcBox.top = 0;
		srcBox.bottom = 1;

		D3D11RenderEngine::d3d11DeviceContext->CopySubresourceRegion(dstD3DBuffer->_hardwareBuffer.get(), 0, dstBytesOffset, 0, 0, _hardwareBuffer.get(), 0, &srcBox);

		return true;
	}

	void D3D11RenderBuffer::CopyStructureCountTo(
		const Ptr<RenderBuffer> & dst,
		uint32_t bytesOffset,
		int32_t firstElement,
		int32_t numElements,
		RenderFormat format,
		uint32_t uavFlag)
	{
		auto uavView = GetUnorderedAccessView(firstElement, numElements, format, uavFlag);
		auto d3d11UAVView = std::static_pointer_cast<D3D11BufferUnorderedAccessView>(uavView);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		auto d3dContex = D3D11RenderEngine::d3d11DeviceContext.get();

		auto d3dDstBuffer = std::dynamic_pointer_cast<D3D11RenderBuffer>(dst);

		d3dContex->CopyStructureCount(d3dDstBuffer->_hardwareBuffer.get(), bytesOffset, d3d11UAVView->hardwareUAV.get());
	}

	Ptr<BufferShaderResourceView> D3D11RenderBuffer::CreateShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat)
	{
		// Init d3d11 srv desc
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));

		if (_desc.bStructured || _desc.bindFlag & BUFFER_BIND_RAW)
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			desc.BufferEx.FirstElement = firstElement;
			desc.BufferEx.NumElements = numElements;
			if (_desc.bindFlag & BUFFER_BIND_RAW)
				desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			else
				desc.BufferEx.Flags = 0;
		}
		else
		{
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.FirstElement = firstElement;
			desc.Buffer.NumElements = numElements;
		}
		desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 srv
		ID3D11ShaderResourceView *pSRV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateShaderResourceView(_hardwareBuffer.get(), &desc, &pSRV);

		auto resultSRV = std::make_shared<D3D11BufferShaderResourceView>();
		resultSRV->hardwareSRV = MakeComShared(pSRV);

		return resultSRV;
	}

	Ptr<BufferUnorderedAccessView> D3D11RenderBuffer::CreateUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat, uint32_t uavFlags)
	{
		// Init d3d11 uav
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

		desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 uav
		ID3D11UnorderedAccessView *pUAV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateUnorderedAccessView(_hardwareBuffer.get(), &desc, &pUAV);

		auto resultUAV = std::make_shared<D3D11BufferUnorderedAccessView>();
		resultUAV->hardwareUAV = MakeComShared(pUAV);

		return resultUAV;
	}

	Ptr<BufferRenderTargetView> D3D11RenderBuffer::CreateRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat)
	{
		// Init d3d11 rtv desc
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
		desc.Format = GetD3DFormat(viewFormat);

		auto re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());

		// Create d3d11 rtv
		ID3D11RenderTargetView *pRTV = nullptr;
		D3D11RenderEngine::d3d11Device->CreateRenderTargetView(_hardwareBuffer.get(), &desc, &pRTV);

		auto resultRTV = std::make_shared<D3D11BufferRenderTargetView>();
		resultRTV->hardwareRTV = MakeComShared(pRTV);

		return resultRTV;
	}

	
	void D3D11VertexInputLayout::Init()
	{
		VertexInputLayout::Init();

		std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementsDescList;

		uint32_t slot = 0;
		for (uint32_t slot = 0; slot < static_cast<uint32_t>(_inputSlotsDesc.size()); ++slot)
		{
			const auto & inputSlotDesc = _inputSlotsDesc[slot];

			for (auto & desc : inputSlotDesc.vertexElementsDesc)
			{
				D3D11_INPUT_ELEMENT_DESC d3dElementDesc;
				d3dElementDesc.SemanticName = desc.name.c_str();
				d3dElementDesc.SemanticIndex = desc.index;
				d3dElementDesc.Format = GetD3DFormat(desc.format);
				d3dElementDesc.AlignedByteOffset = desc.bytesOffset;
				d3dElementDesc.InputSlot = slot;

				if (VERTEX_BUFFER_GEOMETRY == inputSlotDesc.type)
					d3dElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				else
					d3dElementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				d3dElementDesc.InstanceDataStepRate = desc.instanceDataRate;

				d3dElementsDescList.push_back(d3dElementDesc);
			}
		}

		// Create d3d11 input layout
		Ptr<D3D11RenderEngine> re = std::static_pointer_cast<D3D11RenderEngine>(Global::GetRenderEngine());
		ID3D11InputLayout *pInputLayout = nullptr;

		if (d3dElementsDescList.size() > 0)
		{
			D3D11RenderEngine::d3d11Device->CreateInputLayout(
				&d3dElementsDescList[0], static_cast<UINT>(d3dElementsDescList.size()),
				_vertexShader->GetCompiledData().get(), _vertexShader->GetCompiledDataSize(),
				&pInputLayout);
		}

		hardwareInputLayout = MakeComShared(pInputLayout);
	}
}