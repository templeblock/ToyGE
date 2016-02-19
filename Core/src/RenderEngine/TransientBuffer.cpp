#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Platform\Looper.h"

namespace ToyGE
{
	SubAlloc::~SubAlloc()
	{
		Free();
	}

	void SubAlloc::Free()
	{
		if (!bFree)
		{
			if(!transientBuffer.expired())
				transientBuffer.lock()->Free(bytesOffset, bytesSize);
			bFree = true;
		}
	}


	TransientBuffer::~TransientBuffer()
	{
		_frameEventConnection.disconnect();
	}

	void TransientBuffer::Init(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags)
	{
		RenderBufferDesc bufDesc;
		bufDesc.elementSize = elementSize;
		bufDesc.numElements = initNumElements;
		bufDesc.bindFlag = bufferBindFlags;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.bStructured = false;
		if(bufferBindFlags & BUFFER_BIND_VERTEX)
			_buffer = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();
		else
			_buffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		_buffer->SetDesc(bufDesc);
		_buffer->Init(nullptr);

		_bufferData = MakeBufferedDataShared(static_cast<size_t>(bufDesc.elementSize * bufDesc.numElements));

		auto subAlloc = std::make_shared<SubAlloc>();
		subAlloc->bytesOffset = 0;
		subAlloc->bytesSize = elementSize * initNumElements;
		subAlloc->bFree = true;
		subAlloc->transientBuffer = shared_from_this();
		_freeList.push_back(subAlloc);

		_retiredFrames.push_back(CreateRetiredFrame());
	}

	Ptr<SubAlloc> TransientBuffer::Alloc(int32_t bytesSize)
	{
		Ptr<SubAlloc> allocFind;

		auto itr = _freeList.begin();
		while (itr != _freeList.end())
		{
			if ((*itr)->bytesSize >= bytesSize)
			{
				allocFind = std::make_shared<SubAlloc>();
				allocFind->bytesOffset = (*itr)->bytesOffset;
				allocFind->bytesSize = bytesSize;
				allocFind->bFree = false;
				allocFind->transientBuffer = shared_from_this();

				(*itr)->bytesOffset += bytesSize;
				(*itr)->bytesSize -= bytesSize;
				if ((*itr)->bytesSize == 0)
					_freeList.erase(itr);

				break;
			}
			else
				++itr;
		}

		//Not Find
		if (!allocFind)
		{
			auto bufDesc = _buffer->GetDesc();
			bufDesc.numElements *= 2;
			Ptr<RenderBuffer> newBuf;
			if(bufDesc.bindFlag & BUFFER_BIND_VERTEX)
				newBuf = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();
			else
				newBuf = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
			newBuf->SetDesc(bufDesc);
			newBuf->Init(nullptr);
			_buffer->CopyTo(newBuf, 0, 0, static_cast<int32_t>(_buffer->GetDesc().elementSize) * _buffer->GetDesc().numElements);

			auto newBufData = MakeBufferedDataShared(static_cast<size_t>(bufDesc.elementSize * bufDesc.numElements));
			memcpy(newBufData.get(), _bufferData.get(), static_cast<size_t>(_buffer->GetDesc().elementSize * _buffer->GetDesc().numElements));

			auto subAlloc = std::make_shared<SubAlloc>();
			subAlloc->bytesOffset = static_cast<int32_t>(_buffer->GetDesc().elementSize) * _buffer->GetDesc().numElements;
			subAlloc->bytesSize = subAlloc->bytesOffset;
			subAlloc->bFree = true;
			subAlloc->transientBuffer = shared_from_this();
			_freeList.push_back(subAlloc);

			_buffer = newBuf;
			_bufferData = newBufData;

			return Alloc(bytesSize);
		}

		return allocFind;
	}

	void TransientBuffer::Free(int32_t bytesOffset, int32_t bytesSize)
	{
		ToyGE_ASSERT(bytesOffset >= 0);
		ToyGE_ASSERT(bytesSize > 0);
		ToyGE_ASSERT(bytesOffset + bytesSize <= _buffer->GetDataSize());
		ToyGE_ASSERT(_retiredFrames.size() > 0);

		auto allocToFree = std::make_shared<SubAlloc>();
		allocToFree->bytesOffset = bytesOffset;
		allocToFree->bytesSize = bytesSize;
		allocToFree->bFree = true;
		allocToFree->transientBuffer = shared_from_this();
		_retiredFrames.back()->pendingFrees.push_back(allocToFree);
	}

	void TransientBuffer::Update(const Ptr<SubAlloc> & subAlloc, const void * pData, int32_t dataSize, int32_t dstBytesOffset)
	{
		if (dataSize <= 0)
			return;

		TransientBufferUpdateDesc updateDesc;
		updateDesc.subAlloc = subAlloc;
		updateDesc.updateBytesOffset = dstBytesOffset;
		updateDesc.updateDataSize = std::min<int32_t>(dataSize, subAlloc->bytesSize - dstBytesOffset);

		memcpy(_bufferData.get() + updateDesc.subAlloc->bytesOffset + dstBytesOffset, pData, updateDesc.updateDataSize);

		_bufferUpdate.push_back(updateDesc);
	}

	void TransientBuffer::FlushUpdate()
	{
		auto mappedData = _buffer->Map(MAP_WRITE_NO_OVERWRITE);
		for (auto & update : _bufferUpdate)
		{
			memcpy(
				static_cast<uint8_t*>(mappedData.pData) + update.subAlloc->bytesOffset + update.updateBytesOffset,
				_bufferData.get() + update.subAlloc->bytesOffset + update.updateBytesOffset,
				update.updateDataSize);
		}
		_buffer->UnMap();

		_bufferUpdate.clear();
	}

	void TransientBuffer::Register()
	{
		_frameEventConnection = Global::GetLooper()->FrameEvent().connect(std::bind(&TransientBuffer::OnFrame, this));
	}

	void TransientBuffer::OnFrame()
	{
		auto frameItr = _retiredFrames.begin();
		while (frameItr != _retiredFrames.end())
		{
			if ((*frameItr)->IsFrameFinished())
			{
				for (auto & subAlloc : (*frameItr)->pendingFrees)
				{
					DoFree(subAlloc);
				}
				frameItr = _retiredFrames.erase(frameItr);
			}
			else
				++frameItr;
		}

		//Create For Next Frame
		_retiredFrames.push_back(CreateRetiredFrame());
	}

	void TransientBuffer::DoFree(const Ptr<SubAlloc> & subAlloc)
	{
		for (auto & itr = _freeList.begin(); itr != _freeList.end(); ++itr)
		{
			if ((*itr)->bytesOffset > subAlloc->bytesOffset)
			{
				if (subAlloc->bytesOffset + subAlloc->bytesSize == (*itr)->bytesOffset)
				{
					(*itr)->bytesOffset = subAlloc->bytesOffset;
					(*itr)->bytesSize += subAlloc->bytesSize;
				}
				else
				{
					_freeList.insert(itr, subAlloc);
					subAlloc->bFree = true;
				}
				return;
			}
		}

		_freeList.push_back(subAlloc);
	}
}