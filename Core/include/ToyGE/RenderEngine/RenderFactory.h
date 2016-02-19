#pragma once
#ifndef RENDERFACTORY_H
#define RENDERFACTORY_H

#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\BlendState.h"
#include "ToyGE\RenderEngine\DepthStencilState.h"
#include "ToyGE\RenderEngine\RasterizerState.h"
#include "ToyGE\RenderEngine\Sampler.h"
#include "ToyGE\RenderEngine\ShaderProgram.h"

namespace ToyGE
{
	class RenderBuffer;
	class VertexBuffer;
	class VertexInputLayout;
	class ShaderProgram;
	class BlendState;
	class DepthStencilState;
	class RasterizerState;
	class Sampler;
	class Image;
	class Texture;
	class File;
	class TransientBuffer;

	class TOYGE_CORE_API RenderFactory
	{
	public:
		RenderFactory() {};

		virtual ~RenderFactory() = default;

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(ShaderType shaderType) = 0;

		virtual Ptr<Texture> CreateTexture(TextureType type) = 0;

		virtual Ptr<RenderBuffer> CreateBuffer() = 0;

		virtual Ptr<VertexBuffer> CreateVertexBuffer() = 0;

		virtual Ptr<VertexInputLayout> CreateVertexInputLayout() = 0;

		virtual Ptr<TransientBuffer> CreateTransientBuffer() = 0;

		virtual Ptr<BlendState> CreateBlendState() = 0;

		virtual Ptr<DepthStencilState> CreateDepthStencilState() = 0;

		virtual Ptr<RasterizerState> CreateRasterizerState() = 0;

		virtual Ptr<Sampler> CreateSampler() = 0;

		virtual bool CompileShader(
			ShaderType shaderType,
			const String & shaderCode,
			const String & entryName,
			std::shared_ptr<uint8_t> & outCompiledData,
			int32_t & outDataSize) = 0;

		virtual bool GetShaderProgramResourceMap(
			const std::shared_ptr<const uint8_t> & compiledData,
			int32_t dataSize,
			ShaderProgramResourceMap & outResourceMap) = 0;
	};
}

#endif