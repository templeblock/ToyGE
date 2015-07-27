#pragma once
#ifndef RENDERRESOURCE_H
#define RENDERRESOURCE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	enum ResourceBoundType
	{
		RESOURCE_BOUND_SHADER_RESOURCE,
		RESOURCE_BOUND_UAV,
		RESOURCE_BOUND_SHADER_BUFFER,
		RESOURCE_BOUND_RENDER_TARGET,
		RESOURCE_BOUND_DEPTH_STENCIL
	};


	class TOYGE_CORE_API RenderResource : public std::enable_shared_from_this<RenderResource>
	{
	public:
		RenderResource(RenderResourceType type);

		virtual ~RenderResource() = default;

		//render buffer create view
		ResourceView CreateBufferView(
			RenderFormat format,
			int32_t firstElement = 0,
			int32_t numElements = 1,
			uint32_t uavFlags = 0,
			int32_t uavInitalCounts = 0);

		//texture view
		ResourceView CreateTextureView(
			int32_t firstMip = 0,
			int32_t mipLevels = 1,
			int32_t firstArray = 0,
			int32_t arraySize = 1,
			RenderFormat formatHint = RENDER_FORMAT_UNDEFINED,
			int32_t uavInitalCounts = 0);
		//texture view for cube
		ResourceView CreateTextureView_Cube(
			int32_t firstMip,
			int32_t mipLevels,
			int32_t firstFaceOffset,
			int32_t numCubes,
			RenderFormat formatHint = RENDER_FORMAT_UNDEFINED,
			int32_t uavInitalCounts = 0);

		RenderResourceType ResourceType() const
		{
			return _resourceType;
		}
		bool IsBinded() const
		{
			return _isBinded;
		}

		void SetBind(ResourceBoundType bindState, int32_t index, ShaderType shaderType, RenderResourceSubDesc subDesc);
		void SetBind(ResourceBoundType bindState, int32_t index, ShaderType shaderType);
		void SetBind(ResourceBoundType bindState, int32_t index);
		void SetBind(ResourceBoundType bindState);

		void ResetBind()
		{
			_isBinded = false;
		}

		ShaderType BoundShaderType() const
		{
			return _bindShaderType;
		}
		ResourceBoundType BoundState() const
		{
			return _bindState;
		}
		int32_t BoundIndex() const
		{
			return _boundIndex;
		}
		const RenderResourceSubDesc & BoundSubDesc() const
		{
			return _bindSubDesc;
		}

	protected:
		RenderResourceType _resourceType;

		bool _isBinded = false;
		ShaderType _bindShaderType;
		ResourceBoundType _bindState;
		int32_t _boundIndex;
		RenderResourceSubDesc _bindSubDesc;
	};
}

#endif