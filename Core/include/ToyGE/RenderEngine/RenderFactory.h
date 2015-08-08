#pragma once
#ifndef RENDERFACTORY_H
#define RENDERFACTORY_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class RenderInput;
	class RenderBuffer;
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
		friend class Texture;
	public:
		RenderFactory();

		virtual ~RenderFactory() = default;

		virtual Ptr<RenderInput> CreateRenderInput() = 0;

		virtual Ptr<ShaderProgram> CompileShaderProgram(
			ShaderType shaderType,
			const String & shaderText,
			const String & entryName) = 0;

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(
			ShaderType shaderType,
			const std::shared_ptr<uint8_t> & pData,
			size_t dataSize) = 0;

		virtual Ptr<ShaderProgram> CreateShaderShaderProgram(ShaderType shaderType) = 0;

		virtual Ptr<Texture> CreateTexture(const TextureDesc & desc) = 0;

		virtual Ptr<Texture> CreateTexture(const TextureDesc & desc, const std::vector<RenderDataDesc> & initDataList) = 0;

		Ptr<Texture> CreateTexture(const Ptr<Image> & image, uint32_t bindFlag = TEXTURE_BIND_SHADER_RESOURCE, uint32_t cpuAccess = 0);

		Ptr<Texture> CreateTextureFromFile(const Ptr<File> & file, uint32_t bindFlag = TEXTURE_BIND_SHADER_RESOURCE, uint32_t cpuAccess = 0);

		Ptr<Texture> CreateTextureFromFile(const WString & filePath, uint32_t bindFlag = TEXTURE_BIND_SHADER_RESOURCE, uint32_t cpuAccess = 0);

		//Ptr<Texture> CreateTextureAutoGenMips(const Ptr<Image> & image, uint32_t bindFlag = TEXTURE_BIND_SHADER_RESOURCE, uint32_t cpuAccess = 0);

		virtual Ptr<RenderBuffer> CreateBuffer(const RenderBufferDesc & desc, const void * pInitData) = 0;

		/*virtual Ptr<RenderBuffer> CreateBuffer(const RenderBufferDesc & desc, const void * pInitData, 
			VertexBufferType vertexBufferType, const std::vector<VertexElementDesc> & vertexElementDesc) = 0;*/

		virtual Ptr<TransientBuffer> CreateTransientBuffer(int32_t elementSize, int32_t initNumElements, uint32_t bufferBindFlags) = 0;

		virtual Ptr<BlendState> CreateBlendState(const BlendStateDesc & desc) = 0;

		Ptr<BlendState> GetBlendStatePooled(const BlendStateDesc & desc);

		virtual Ptr<DepthStencilState> CreateDepthStencilState(const DepthStencilStateDesc & desc) = 0;

		Ptr<DepthStencilState> GetDepthStencilStatePooled(const DepthStencilStateDesc & desc);

		virtual Ptr<RasterizerState> CreateRasterizerState(const RasterizerStateDesc & desc) = 0;

		Ptr<RasterizerState> GetRasterizerStatePooled(const RasterizerStateDesc & desc);

		virtual Ptr<Sampler> CreateSampler(const SamplerDesc & desc) = 0;

		Ptr<Sampler> GetSamplerPooled(const SamplerDesc & desc);

		virtual void InitShaderProgramReflectInfo(const Ptr<ShaderProgram> & shaderProgram) = 0;

		Ptr<Texture> GetTexturePooled(const TextureDesc & desc);

		void ClearTexturePool();

	private:
		std::map<uint64_t, Ptr<Sampler>> _samplersPool;
		std::map<uint64_t, Ptr<BlendState>> _blendStatePool;
		std::map<uint64_t, Ptr<DepthStencilState>> _depthStendilStatePool;
		std::map<uint64_t, Ptr<RasterizerState>> _rasterizerStatePool;
		std::map<uint64_t, std::vector<Ptr<Texture>>> _texturePoolIdle;

		void ReleaseTextureToPool(Ptr<Texture> & tex);
	};
}

#endif