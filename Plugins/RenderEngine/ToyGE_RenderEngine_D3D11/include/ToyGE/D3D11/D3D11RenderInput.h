#pragma once
#ifndef D3D11RENDERINPUT_H
#define D3D11RENDERINPUT_H

#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

namespace ToyGE
{
	class D3D11VertexShaderProgram;

	class D3D11RenderInput : public RenderInput
	{
	public:
		/*D3D11RenderInput(const std::vector<Ptr<RenderBuffer>> & vertices, const Ptr<RenderBuffer> & indices,
			PrimitiveTopology topology = PRIMITIVE_TOPOLOGY_TRIANGLELIST);*/

		const Ptr<ID3D11InputLayout> & AcquireRawD3DInputLayout(const Ptr<D3D11VertexShaderProgram> & vertexShader);

	private:
		std::map<uint32_t, Ptr<ID3D11InputLayout>> _inputLayoutMap;

	};
}

#endif