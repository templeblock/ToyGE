#pragma once
#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderResource.h"

namespace ToyGE
{

	class TOYGE_CORE_API RenderBuffer : public RenderResource
	{
	public:
		RenderBuffer(const RenderBufferDesc & desc, const void *pInitData);
		//RenderBuffer(
		//	const RenderBufferDesc & desc,
		//	const void * pInitData,
		//	VertexBufferType vertexBufferType,
		//	const std::vector<VertexElementDesc> & vertexElementDesc);

		virtual ~RenderBuffer() = default;

		const RenderBufferDesc & Desc() const
		{
			return _desc;
		}

		CLASS_GET(VertexBufferType, VertexBufferType, _vertexBufferType);
		CLASS_SET(VertexBufferType, VertexBufferType, _vertexBufferType);

		CLASS_GET(VertexElementsDesc, std::vector<VertexElementDesc>, _vertexElementsDesc);
		CLASS_SET(VertexElementsDesc, std::vector<VertexElementDesc>, _vertexElementsDesc);

		virtual RenderDataDesc Map(MapType mapFlag) = 0;

		virtual void UnMap() = 0;

		virtual bool CopyTo(const Ptr<RenderBuffer> & dst, int32_t dstBytesOffset, int32_t srcBytesOffset, int32_t cpyBytesSize) const = 0;

		virtual void CopyStructureCountTo(
			const Ptr<RenderBuffer> & dst,
			uint32_t bytesOffset,
			int32_t firstElement,
			int32_t numElements,
			RenderFormat format,
			uint32_t uavFlag) = 0;

	protected:
		RenderBufferDesc _desc;
		VertexBufferType _vertexBufferType;
		std::vector<VertexElementDesc> _vertexElementsDesc;
	};
}

#endif
