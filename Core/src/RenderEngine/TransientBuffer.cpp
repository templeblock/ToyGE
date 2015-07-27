#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Platform\Looper.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	TransientBuffer::TransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags)
	{
		RenderBufferDesc bufDesc;
		bufDesc.elementSize = static_cast<size_t>(elementSize);
		bufDesc.numElements = initNumElements;
		bufDesc.bindFlag = bufferBindFlags;
		bufDesc.cpuAccess = CPU_ACCESS_WRITE;
		bufDesc.structedByteStride = 0;
		_buffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, nullptr);

		_bufferData = MakeBufferedDataShared(static_cast<size_t>(bufDesc.elementSize * bufDesc.numElements));

		auto subAlloc = std::make_shared<SubAlloc>();
		subAlloc->bytesOffset = 0;
		subAlloc->bytesSize = elementSize * initNumElements;
		_freeList.push_back(subAlloc);
	}

	TransientBuffer::~TransientBuffer()
	{
		//Global::GetLooper()->FrameEvent().disconnect(std::bind(&TransientBuffer::OnFrame, this));
		_frameEventConnection.disconnect();
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
			auto bufDesc = _buffer->Desc();
			bufDesc.numElements *= 2;
			auto newBuf = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(bufDesc, nullptr);
			_buffer->CopyTo(newBuf, 0, 0, static_cast<int32_t>(_buffer->Desc().elementSize) * _buffer->Desc().numElements);

			auto newBufData = MakeBufferedDataShared(static_cast<size_t>(bufDesc.elementSize * bufDesc.numElements));
			memcpy(newBufData.get(), _bufferData.get(), static_cast<size_t>(_buffer->Desc().elementSize * _buffer->Desc().numElements));

			auto subAlloc = std::make_shared<SubAlloc>();
			subAlloc->bytesOffset = static_cast<int32_t>(_buffer->Desc().elementSize) * _buffer->Desc().numElements;
			subAlloc->bytesSize = subAlloc->bytesOffset;
			_freeList.push_back(subAlloc);

			_buffer = newBuf;
			_bufferData = newBufData;

			return Alloc(bytesSize);
		}

		return allocFind;
	}

	void TransientBuffer::Free(const Ptr<SubAlloc> & subAlloc)
	{
		if (!subAlloc)
			return;

		_retiredFrames.back()->pendingFrees.push_back(subAlloc);
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

	void TransientBuffer::UpdateFinish()
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
				}
				return;
			}
		}

		_freeList.push_back(subAlloc);
	}
}