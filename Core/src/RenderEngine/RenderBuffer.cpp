#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	void RenderBuffer::Init(const void * pInitData, bool bCacheData)
	{
		RenderResource::Init();

		if (bCacheData)
		{
			size_t dataSize = static_cast<size_t>(_desc.elementSize * _desc.numElements);
			_cachedData = MakeBufferedDataShared(dataSize);
			memcpy_s(_cachedData.get(), dataSize, pInitData, dataSize);
		}
	}

	void RenderBuffer::Release()
	{
		RenderResource::Release();

		for (auto & view : _srvCache)
			view->Release();
		_srvCache.clear();
		for (auto & view : _uavCache)
			view->Release();
		_uavCache.clear();
		for (auto & view : _rtvCache)
			view->Release();
		_rtvCache.clear();
	}

	bool RenderBuffer::Dump(void * outDumpBuffer)
	{
		if (!IsInit())
			return false;

		auto dumpBuffer = shared_from_this()->Cast<RenderBuffer>();
		if (!(_desc.cpuAccess & CPU_ACCESS_READ))
		{
			auto dumpBufferDesc = _desc;
			dumpBufferDesc.bindFlag = 0;
			dumpBufferDesc.cpuAccess = CPU_ACCESS_READ;
			dumpBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
			dumpBuffer->SetDesc(dumpBufferDesc);
			dumpBuffer->Init();
			if (!this->CopyTo(dumpBuffer, 0, 0, GetDataSize()))
				return false;
		}

		auto mappedData = dumpBuffer->Map(MAP_READ);
		memcpy(outDumpBuffer, mappedData.pData, GetDataSize());
		dumpBuffer->UnMap();
		return true;
	}

	Ptr<BufferShaderResourceView> RenderBuffer::GetShaderResourceView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat)
	{
		if (numElements == 0)
			numElements = _desc.numElements;

		for (auto & view : _srvCache)
		{
			if (view->firstElement == firstElement &&
				view->numElements == numElements &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateShaderResourceView(firstElement, numElements, viewFormat);
		newView->firstElement = firstElement;
		newView->numElements = numElements;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_srvCache.push_back(newView);

		return newView;
	}

	Ptr<BufferUnorderedAccessView> RenderBuffer::GetUnorderedAccessView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat, uint32_t uavFlags)
	{
		if (numElements == 0)
			numElements = _desc.numElements;

		for (auto & view : _uavCache)
		{
			if (view->firstElement == firstElement &&
				view->numElements == numElements &&
				view->viewFormat == viewFormat &&
				view->uavFlags == uavFlags)
				return view;
		}

		auto newView = CreateUnorderedAccessView(firstElement, numElements, viewFormat, uavFlags);
		newView->firstElement = firstElement;
		newView->numElements = numElements;
		newView->viewFormat = viewFormat;
		newView->uavFlags = uavFlags;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_uavCache.push_back(newView);

		return newView;
	}

	Ptr<BufferRenderTargetView> RenderBuffer::GetRenderTargetView(int32_t firstElement, int32_t numElements, RenderFormat viewFormat)
	{
		if (numElements == 0)
			numElements = _desc.numElements;

		for (auto & view : _rtvCache)
		{
			if (view->firstElement == firstElement &&
				view->numElements == numElements &&
				view->viewFormat == viewFormat)
				return view;
		}

		auto newView = CreateRenderTargetView(firstElement, numElements, viewFormat);
		newView->firstElement = firstElement;
		newView->numElements = numElements;
		newView->viewFormat = viewFormat;
		newView->SetResource(shared_from_this()->Cast<RenderResource>());
		_rtvCache.push_back(newView);

		return newView;
	}


	std::map<Ptr<ShaderProgram>, VertexInputLayout::VertexInputLayoutList> VertexInputLayout::_vertexInputLayoutMap;

	void GetVertexInputSlotsDesc(const std::vector<Ptr<VertexBuffer>> & vbs, std::vector<VertexInputSlotDesc> & inputSlotsDesc)
	{
		for (auto & vb : vbs)
		{
			if(vb)
				inputSlotsDesc.push_back({ vb->GetElementsDesc(), vb->GetType() });
		}
	}

	Ptr<VertexInputLayout> VertexInputLayout::GetVertexInputLayout(const Ptr<ShaderProgram> & vertexShader, const std::vector<Ptr<VertexBuffer>> & vertexBuffers)
	{
		std::vector<VertexInputSlotDesc> inputSlotSort;
		GetVertexInputSlotsDesc(vertexBuffers, inputSlotSort);
		for(auto & slot : inputSlotSort)
			slot.Sort();

		auto & inputLayoutList = _vertexInputLayoutMap[vertexShader];
		for (auto & layout : inputLayoutList)
		{
			if (layout->_inputSlotsDesc == inputSlotSort)
				return layout;
		}

		// Not found, create new
		auto newLayout = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexInputLayout();
		newLayout->_vertexShader = vertexShader;
		newLayout->_inputSlotsDesc = inputSlotSort;
		newLayout->Init();

		inputLayoutList.push_back(newLayout);

		return newLayout;
	}


	void VertexBufferBuilder::AddElementDesc(const String & name, int32_t index, RenderFormat format, int32_t instanceRate)
	{
		VertexElementDesc desc;
		desc.name = name;
		desc.index = index;
		desc.format = format;
		desc.instanceDataRate = instanceRate;
		desc.bytesOffset = vertexSize;
		desc.bytesSize = GetRenderFormatNumBits(format) / 8;
		vertexElementsDesc.push_back(desc);
		vertexSize += desc.bytesSize;
	}

	void VertexBufferBuilder::SetNumVertices(int32_t numVertices)
	{
		this->numVertices = numVertices;
		dataBuffer = MakeBufferedDataShared(static_cast<size_t>(numVertices * vertexSize));
		pData = dataBuffer.get();
	}

	Ptr<VertexBuffer> VertexBufferBuilder::Finish()
	{
		auto vb = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();
		RenderBufferDesc desc;
		desc.bindFlag = BUFFER_BIND_VERTEX;
		desc.cpuAccess = 0;
		desc.elementSize = vertexSize;
		desc.numElements = numVertices;
		desc.bStructured = false;
		vb->SetDesc(desc);

		vb->SetType(type);
		vb->SetElementsDesc(vertexElementsDesc);
		vb->Init(dataBuffer.get());

		return vb;
	}
}
