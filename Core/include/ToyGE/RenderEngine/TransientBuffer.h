#pragma once
#ifndef TRANSIENTBUFFER_H
#define TRANSIENTBUFFER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "boost\noncopyable.hpp"
#include "boost\signals2.hpp"

namespace ToyGE
{
	class SubAlloc : public std::enable_shared_from_this<SubAlloc>
	{
	public:
		int32_t bytesOffset;
		int32_t bytesSize;
		bool	bFree = true;

		std::weak_ptr<class TransientBuffer> transientBuffer;

		~SubAlloc();

		void Free();
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

	class TOYGE_CORE_API TransientBuffer : public std::enable_shared_from_this<TransientBuffer>, public boost::noncopyable
	{
		friend class SubAlloc;
	public:
		TransientBuffer() {};

		virtual ~TransientBuffer();

		void Init(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags);

		CLASS_GET(Buffer, Ptr<RenderBuffer>, _buffer);

		Ptr<SubAlloc> Alloc(int32_t bytesSize);


		void Update(const Ptr<SubAlloc> & subAlloc, const void * pData, int32_t dataSize, int32_t dstBytesOffset);

		void FlushUpdate();

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

		void Free(int32_t bytesOffset, int32_t bytesSize);

		void DoFree(const Ptr<SubAlloc> & subAlloc);
	};
}

#endif