#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	RenderInput::RenderInput()
		: _topology(PRIMITIVE_TOPOLOGY_UNDEFINED),
		_verticesBytesOffsets(_vertices.size(), 0),
		_vertexStart(0),
		_indicesBytesOffset(0),
		_indexStart(0)
	{

	}

	int32_t RenderInput::NumVertices() const
	{
		for (auto & vb : _vertices)
		{
			if (VERTEX_BUFFER_GEOMETRY == vb->GetVertexBufferType())
				return vb->Desc().numElements;
		}

		return 0;
	}

	int32_t RenderInput::NumIndices() const
	{
		if (_indices)
			return _indices->Desc().numElements;
		else
			return 0;
	}



	Ptr<RenderInput> CommonInput::_quadInput;

	void CommonInput::Clear()
	{
		_quadInput.reset();
	}

	const Ptr<RenderInput> & CommonInput::QuadInput()
	{
		if (!_quadInput)
		{
			XMFLOAT2 quadVertices[] =
			{
				{ -1.0f,  1.0f },
				{  1.0f,  1.0f },
				{ -1.0f, -1.0f },
				{  1.0f, -1.0f }
			};

			uint32_t quadIndices[] =
			{
				0, 1, 2,
				1, 3, 2
			};

			RenderBufferDesc desc;
			desc.bindFlag = BUFFER_BIND_VERTEX;
			desc.cpuAccess = 0;
			desc.elementSize = sizeof(XMFLOAT2);
			desc.numElements = static_cast<int32_t>(_countof(quadVertices));
			desc.structedByteStride = 0;

			std::vector<VertexElementDesc> verticesEleDesc;
			VertexElementDesc eleDesc;
			eleDesc.format = RENDER_FORMAT_R32G32_FLOAT;
			eleDesc.index = 0;
			eleDesc.instanceDataRate = 0;
			eleDesc.name = "POSITION";
			verticesEleDesc.push_back(eleDesc);

			auto vb = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(desc, quadVertices);
			vb->SetVertexBufferType(VERTEX_BUFFER_GEOMETRY);
			vb->SetVertexElementsDesc(verticesEleDesc);

			desc.cpuAccess = CPU_ACCESS_WRITE;
			desc.elementSize = sizeof(float2);
			desc.numElements = 4;
			
			verticesEleDesc.clear();
			eleDesc.format = RENDER_FORMAT_R32G32_FLOAT;
			eleDesc.name = "TEXCOORD";
			verticesEleDesc.push_back(eleDesc);

			auto uvBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(desc, nullptr);
			uvBuffer->SetVertexBufferType(VERTEX_BUFFER_GEOMETRY);
			uvBuffer->SetVertexElementsDesc(verticesEleDesc);

			desc.bindFlag = BUFFER_BIND_INDEX;
			desc.elementSize = sizeof(uint32_t);
			desc.numElements = static_cast<int32_t>(_countof(quadIndices));

			auto ib = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(desc, quadIndices);

			_quadInput = Global::GetRenderEngine()->GetRenderFactory()->CreateRenderInput();
			_quadInput->SetVerticesBuffers({ vb, uvBuffer });
			_quadInput->SetIndicesBuffers(ib);
			_quadInput->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		return _quadInput;
	}
}