#pragma once
#ifndef TRANSIENTBUFFER_H
#define TRANSIENTBUFFER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "boost\noncopyable.hpp"
#include "boost\signals2.hpp"

namespace ToyGE
{
	class SubAlloc
	{
	public:
		int32_t bytesOffset;
		int32_t bytesSize;
	};
	//using Ptr<SubAlloc> = std::shared_ptr < SubAlloc > ;


	class RetiredFrame
	{
	public:
		virtual ~RetiredFrame() = default;

		std::list<Ptr<SubAlloc>> pendingFrees;

		virtual bool IsFrameFinished() = 0;
	};
	using RetiredFramePtr = std::shared_ptr < RetiredFrame >;


	struct TransientBufferUpdateDesc
	{
		Ptr<SubAlloc> subAlloc;
		int32_t updateBytesOffset;
		int32_t updateDataSize;
	};

	class RenderBuffer;

	class TOYGE_CORE_API TransientBuffer : public boost::noncopyable
	{
	public:
		TransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags);

		virtual ~TransientBuffer();

		CLASS_GET(Buffer, Ptr<RenderBuffer>, _buffer);

		Ptr<SubAlloc> Alloc(int32_t bytesSize);

		void Free(const Ptr<SubAlloc> & subAlloc);

		//void UpdateStart();

		void Update(const Ptr<SubAlloc> & subAlloc, const void * pData, int32_t dataSize, int32_t dstBytesOffset);

		void UpdateFinish();

		void Register();

	protected:
		virtual RetiredFramePtr CreateRetiredFrame() const = 0;

	private:
		Ptr<RenderBuffer> _buffer;
		std::shared_ptr<uint8_t> _bufferData;
		std::vector<TransientBufferUpdateDesc> _bufferUpdate;
		std::list<Ptr<SubAlloc>> _freeList;
		std::list<RetiredFramePtr> _retiredFrames;
		boost::signals2::connection _frameEventConnection;

		void OnFrame();

		void DoFree(const Ptr<SubAlloc> & subAlloc);
	};
}

#endif