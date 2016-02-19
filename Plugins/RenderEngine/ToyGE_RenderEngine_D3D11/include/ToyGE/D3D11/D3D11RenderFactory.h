#pragma once
#ifndef D3D11RENDERFACTORY_H
#define D3D11RENDERFACTORY_H

#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"

namespace ToyGE
{
	class D3D11RenderFactory : public RenderFactory
	{
	public:
		D3D11RenderFactory();

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(ShaderType shaderType) override;

		virtual Ptr<Texture> CreateTexture(TextureType type) override;

		virtual Ptr<RenderBuffer> CreateBuffer() override;

		virtual Ptr<VertexBuffer> CreateVertexBuffer() override;

		virtual Ptr<VertexInputLayout> CreateVertexInputLayout() override;

		virtual Ptr<TransientBuffer> CreateTransientBuffer() override;

		virtual Ptr<BlendState> CreateBlendState() override;

		virtual Ptr<DepthStencilState> CreateDepthStencilState() override;

		virtual Ptr<RasterizerState> CreateRasterizerState() override;

		virtual Ptr<Sampler> CreateSampler() override;

		virtual bool CompileShader(
			ShaderType shaderType,
			const String & shaderCode,
			const String & entryName,
			std::shared_ptr<uint8_t> & outCompiledData,
			int32_t & outDataSize) override;

		virtual bool GetShaderProgramResourceMap(
			const std::shared_ptr<const uint8_t> & compiledData,
			int32_t dataSize,
			ShaderProgramResourceMap & outResourceMap) override;

	private:
		D3DCompileFunc _compileFunc;
		D3DReflectFunc _reflectFunc;
	};
}

#endif