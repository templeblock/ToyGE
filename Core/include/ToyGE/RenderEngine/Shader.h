#pragma once
#ifndef SHADER_H
#define SHADER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class ShaderProgram;
	class RenderBuffer;
	class Sampler;

	class TOYGE_CORE_API Shader : public std::enable_shared_from_this<Shader>
	{
		friend class RenderContext;

	public:
		//Shader(const Ptr<ShaderProgram> & shaderProgram);

		virtual ~Shader() = default;

		void SetProgram(const Ptr<ShaderProgram> & program);

		CLASS_GET(Program, Ptr<ShaderProgram>, _shaderProgram);

		int32_t NumShaderResources() const
		{
			return static_cast<int32_t>(_srs.size());
		}

		void SetShaderResource(int32_t index, const ResourceView & sr)
		{
			_srs[index] = sr;
		}

		const ResourceView & GetShaderResource(int32_t index) const
		{
			return _srs[index];
		}

		int32_t NumConstantBuffers() const
		{
			return static_cast<int32_t>(_cbs.size());
		}

		void SetConstantBuffer(int32_t index, const Ptr<RenderBuffer> & buf)
		{
			_cbs[index] = buf;
		}

		const Ptr<RenderBuffer> & GetConstantBuffer(int32_t index) const
		{
			return _cbs[index];
		}

		int32_t NumSamplers() const
		{
			return static_cast<int32_t>(_samplers.size());
		}

		void SetSampler(int32_t index, const Ptr<Sampler> & sampler)
		{
			_samplers[index] = sampler;
		}

		const Ptr<Sampler> & GetSampler(int32_t index) const
		{
			return _samplers[index];
		}

		int32_t NumUAVs() const
		{
			return static_cast<int32_t>(_uavs.size());
		}

		void SetUAV(int32_t index, const ResourceView & uav)
		{
			_uavs[index] = uav;
		}

		const ResourceView & GetUAV(int32_t index) const
		{
			return _uavs[index];
		}
			

		//void InitReflectionInfo();
		//const ShaderReflectionInfo & ReflectionInfo() const
		//{
		//	return _reflectInfo;
		//}

		//ShaderType Type()
		//{
		//	return _shaderProgram->Type();
		//}

		/*void SetShaderResourceIndex(const ResourceView & res, uint32_t index)
		{
			ToyGE_ASSERT(index < _srs.size());
			_srs[index] = res;
		}
		int32_t NumShaderResources() const
		{
			return static_cast<int32_t>(_srs.size());
		}
		void ResizeNumShaderResources(int32_t num)
		{
			_srs.resize(num);
		}
		const ResourceView & GetShaderResource(int32_t index) const
		{
			return _srs[index];
		}

		void SetShaderBufferIndex(const Ptr<RenderBuffer> & buffer, int32_t index)
		{
			_buffers[index] = buffer;
		}
		int32_t NumBuffers() const
		{
			return static_cast<int32_t>(_buffers.size());
		}
		void ResizeNumShaderBuffers(int32_t num)
		{
			_buffers.resize(num);
		}
		const Ptr<RenderBuffer> & GetBuffer(int32_t index) const
		{
			return _buffers[index];
		}

		void SetSamplerIndex(const Ptr<Sampler> & sampler, int32_t index)
		{
			_samplers[index] = sampler;
		}
		int32_t NumSamplers() const
		{
			return static_cast<int32_t>(_samplers.size());
		}
		void ResizeNumSamplers(int32_t num)
		{
			_samplers.resize(num);
		}
		const Ptr<Sampler> & GetSampler(int32_t index) const
		{
			return _samplers[index];
		}

		void SetUAVIndex(const ResourceView & uav, int32_t index)
		{
			_uavs[index] = uav;
		}
		int32_t NumUAVs() const
		{
			return static_cast<int32_t>(_uavs.size());
		}
		void ResizeNumUAVs(int32_t num)
		{
			_uavs.resize(num);
		}
		const ResourceView & GetUAV(int32_t index) const
		{
			return _uavs[index];
		}*/

	protected:
		Ptr<ShaderProgram> _shaderProgram;
		std::vector<ResourceView> _srs;
		std::vector<Ptr<RenderBuffer>> _cbs;
		std::vector<Ptr<Sampler>> _samplers;
		std::vector<ResourceView> _uavs;

		//ShaderReflectionInfo _reflectInfo;
	};
}

#endif