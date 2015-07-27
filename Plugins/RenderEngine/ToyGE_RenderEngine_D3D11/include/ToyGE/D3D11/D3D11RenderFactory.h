#pragma once
#ifndef D3D11RENDERFACTORY_H
#define D3D11RENDERFACTORY_H

#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"

namespace ToyGE
{
	class D3D11RenderFactory : public RenderFactory
	{
	public:
		D3D11RenderFactory();

		Ptr<RenderInput> CreateRenderInput() override;

		/*ShaderPtr
			CreateShader
			(ShaderType shaderType, const void *pData, size_t dataSize, const String & entryName) override;
		ShaderPtr
			CreateShaderFromBin
			(ShaderType shaderType, const std::shared_ptr<uint8_t> & pData, size_t dataSize) override;*/

		virtual Ptr<ShaderProgram> CompileShaderProgram(
			ShaderType shaderType,
			const String & shaderText,
			const String & entryName) override;

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(
			ShaderType shaderType,
			const std::shared_ptr<uint8_t> & compiledData,
			size_t dataSize) override;

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(ShaderType shaderType) override;

		Ptr<Texture> CreateTexture(const TextureDesc & desc) override;

		Ptr<Texture> CreateTexture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList) override;

		Ptr<RenderBuffer> CreateBuffer(const RenderBufferDesc & desc, const void * pInitData) override;

		/*Ptr<RenderBuffer> CreateBuffer(const RenderBufferDesc & desc, const void * pInitData,
			VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc) override;*/

		Ptr<TransientBuffer> CreateTransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags) override;

		Ptr<BlendState> CreateBlendState(const BlendStateDesc & desc) override;

		Ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc & desc) override;

		Ptr<RasterizerState> CreateRasterizerState(const RasterizerStateDesc & desc) override;

		Ptr<Sampler> CreateSampler(const SamplerDesc & desc) override;

		void InitShaderProgramReflectInfo(const Ptr<ShaderProgram> & shaderProgram) override;

	private:
		D3DCompileFunc _compileFunc;
		D3DReflectFunc _reflectFunc;
	};
}

#endif