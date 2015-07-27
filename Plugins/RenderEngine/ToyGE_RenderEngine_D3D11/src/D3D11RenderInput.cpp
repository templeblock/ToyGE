#include "ToyGE\D3D11\D3D11RenderInput.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	//D3D11RenderInput::D3D11RenderInput(const std::vector<Ptr<RenderBuffer>> & vertices, const Ptr<RenderBuffer> & indices, PrimitiveTopology topology)
	//	: RenderInput(vertices, indices, topology)
	//{
	//	//_d3dTopology = GetD3DPrimitiveTopology(topology);
	//}

	const Ptr<ID3D11InputLayout> & D3D11RenderInput::AcquireRawD3DInputLayout(const Ptr<D3D11VertexShaderProgram> & vertexShader)
	{
		static Ptr<ID3D11InputLayout> empty;
		if (_vertices.size() == 0)
			return empty;

		auto itr = _inputLayoutMap.find(vertexShader->VertexShaderProgramID());

		if (itr != _inputLayoutMap.end())
		{
			return itr->second;
		}
		else
		{
			//ToyGE_ASSERT_MSG(_vertices.size() > 0, "No Vertex Buffers!");

			//Create inputlayout
			std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementDescList;
			uint32_t slot = 0;
			for (uint32_t slot = 0; slot < static_cast<uint32_t>(_vertices.size()); ++slot)
			{
				/*const std::vector<VertexElementDesc> * elementsDesc;
				if (_verticesElementDesc[slot].size() > 0)
					elementsDesc = &_verticesElementDesc[slot];
				else
					elementsDesc = &_vertices[slot]->GetVertexElementsDesc();*/

				const auto & elementsDesc = _vertices[slot]->GetVertexElementsDesc();

				uint32_t bytesOffset = 0;
				for (auto & desc : elementsDesc)
				{
					D3D11_INPUT_ELEMENT_DESC d3dElementDesc;
					d3dElementDesc.SemanticName = desc.name.c_str();
					d3dElementDesc.SemanticIndex = desc.index;
					d3dElementDesc.Format = GetD3DFormat(desc.format);
					d3dElementDesc.AlignedByteOffset = bytesOffset;

					uint32_t fmtBits = GetRenderFormatNumBits(desc.format);
					ToyGE_ASSERT_MSG(fmtBits % 8 == 0, "Format bits error!");
					bytesOffset += fmtBits / 8;

					d3dElementDesc.InputSlot = slot;
					if (VERTEX_BUFFER_GEOMETRY == _vertices[slot]->GetVertexBufferType())
						d3dElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					else
						d3dElementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
					d3dElementDesc.InstanceDataStepRate = desc.instanceDataRate;

					d3dElementDescList.push_back(d3dElementDesc);
				}
			}

			Ptr<D3D11RenderEngine> re = std::static_pointer_cast<D3D11RenderEngine>( Global::GetRenderEngine() );
			ID3D11InputLayout *pInputLayout = nullptr;
			re->RawD3DDevice()->CreateInputLayout(
				&d3dElementDescList[0], static_cast<UINT>(d3dElementDescList.size()),
				vertexShader->GetCompiledData().get(), vertexShader->GetCompiledDataSize(),
				&pInputLayout);

			return _inputLayoutMap.insert( std::make_pair( vertexShader->VertexShaderProgramID(), MakeComShared(pInputLayout) ) ).first->second;
		}
	}
}