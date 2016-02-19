#pragma once
#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "ToyGE\RenderEngine\RenderResource.h"

namespace ToyGE
{
	struct BoundCBVariable
	{
		String	name;
		int32_t bytesOffset;
		int32_t bytesSize;
	};

	struct BoundConstantBuffer
	{
		String	name;
		int32_t boundIndex;
		int32_t bytesSize;
		std::vector<BoundCBVariable> variables;
	};

	struct BoundResource
	{
		String	name;
		int32_t boundIndex;
	};

	struct ShaderProgramResourceMap
	{
		std::vector<BoundConstantBuffer>	constantBuffers;
		std::vector<BoundResource>			srvs;
		std::vector<BoundResource>			uavs;
		std::vector<BoundResource>			samplers;
	};

	class TOYGE_CORE_API ShaderProgram : public RenderResource
	{
		friend class Shader;
		friend class ShaderMetaType;

	public:
		ShaderProgram(ShaderType type) : RenderResource(RenderResourceType::RRT_SHADER), _type(type) {};

		virtual ~ShaderProgram() = default;

		virtual void Init() override;

		CLASS_SET(Type, ShaderType, _type);
		CLASS_GET(Type, ShaderType, _type);

		CLASS_GET(ResourceMap, ShaderProgramResourceMap, _resourceMap);
		CLASS_SET(ResourceMap, ShaderProgramResourceMap, _resourceMap);

		std::shared_ptr<const uint8_t> GetCompiledData() const
		{
			return _compiledData;
		}

		int32_t GetCompiledDataSize() const
		{
			return _compiledDataSize;
		}

	protected:
		ShaderType _type;
		ShaderProgramResourceMap _resourceMap;
		std::shared_ptr<uint8_t> _compiledData;
		int32_t _compiledDataSize;
	};
}

#endif