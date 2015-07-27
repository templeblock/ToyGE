#pragma once
#ifndef RENDERINPUT_H
#define RENDERINPUT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class RenderBuffer;

	class TOYGE_CORE_API RenderInput
	{
	public:
		RenderInput();

		virtual ~RenderInput() = default;

		CLASS_GET(PrimitiveTopology, PrimitiveTopology, _topology);
		CLASS_SET(PrimitiveTopology, PrimitiveTopology, _topology);

		//void SetVerticesElementDesc(int32_t slotIndex, const std::vector<VertexElementDesc> & elementsDesc)
		//{
		//	_verticesElementDesc[slotIndex] = elementsDesc;
		//}

		//const std::vector<VertexElementDesc> & GetVerticesElementDesc(int32_t slotIndex) const
		//{
		//	return _verticesElementDesc[slotIndex];
		//}

		void SetVerticesBuffers(const std::vector<Ptr<RenderBuffer>> & vertices)
		{
			_vertices = vertices;
			_verticesBytesOffsets.clear();
			_verticesBytesOffsets.resize(vertices.size());
			//_verticesElementDesc.clear();
			//_verticesElementDesc.resize(vertices.size());
		}

		const std::vector<Ptr<RenderBuffer>> & GetVerticesBuffers() const
		{
			return _vertices;
		}

		void SetIndicesBuffers(const Ptr<RenderBuffer> & indices)
		{
			_indices = indices;
		}

		const Ptr<RenderBuffer> & GetIndicesBuffer() const
		{
			return _indices;
		}

		int32_t NumVertices() const;

		int32_t NumIndices() const;

		void SetVerticesBytesOffset(int32_t index, size_t offset)
		{
			_verticesBytesOffsets[index] = offset;
		}

		size_t GetVerticesBytesOffset(int32_t index) const
		{
			return _verticesBytesOffsets[index];
		}

		void SetIndicesBytesOffset(size_t offset)
		{
			_indicesBytesOffset = offset;
		}

		size_t GetIndicesBytesOffset() const
		{
			return _indicesBytesOffset;
		}

		CLASS_GET(VertexStart, int32_t, _vertexStart);
		CLASS_SET(VertexStart, int32_t, _vertexStart);

		CLASS_GET(IndexStart, int32_t, _indexStart);
		CLASS_SET(IndexStart, int32_t, _indexStart);

	protected:
		PrimitiveTopology _topology;
		//std::vector< std::vector<VertexElementDesc> > _verticesElementDesc;
		std::vector<Ptr<RenderBuffer>> _vertices;
		std::vector<size_t> _verticesBytesOffsets;
		int32_t _vertexStart;
		Ptr<RenderBuffer> _indices;
		size_t _indicesBytesOffset;
		int32_t _indexStart;
	};


	class TOYGE_CORE_API CommonInput
	{
	public:
		static const Ptr<RenderInput> & QuadInput();

		static void Clear();
	private:
		static Ptr<RenderInput> _quadInput;
	};
}

#endif