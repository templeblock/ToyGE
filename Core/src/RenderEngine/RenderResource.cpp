#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\Texture.h"

namespace ToyGE
{
	RenderResource::RenderResource(RenderResourceType type)
		: _resourceType(type)
	{

	}

	//render buffer create view
	ResourceView RenderResource::CreateBufferView(RenderFormat format, int32_t firstElement, int32_t numElements, uint32_t uavFlags, int32_t uavInitalCounts)
	{
		ToyGE_ASSERT(firstElement + numElements <= static_cast<RenderBuffer*>(this)->Desc().numElements);
		ToyGE_ASSERT(RENDER_RESOURCE_BUFFER == ResourceType());

		ResourceView view;
		view.resource = shared_from_this();
		view.subDesc.bufferDesc.firstElement = firstElement;
		view.subDesc.bufferDesc.numElements = numElements == 0 ? static_cast<RenderBuffer*>(this)->Desc().numElements : numElements;
		view.subDesc.bufferDesc.uavFlags = uavFlags;
		view.subDesc.bufferDesc.uavInitalCounts = uavInitalCounts;
		view.formatHint = format;

		return view;
	}

	//texture view
	ResourceView RenderResource::CreateTextureView(
		int32_t firstMip,
		int32_t mipLevels,
		int32_t firstArray,
		int32_t arraySize,
		RenderFormat formatHint,
		int32_t uavInitalCounts)
	{
		ToyGE_ASSERT(firstMip + mipLevels <= static_cast<Texture*>(this)->Desc().mipLevels);
		auto & texDesc = static_cast<Texture*>(this)->Desc();
		if (TEXTURE_CUBE == texDesc.type)
			ToyGE_ASSERT(firstArray + arraySize <= static_cast<Texture*>(this)->Desc().arraySize * 6);
		else if (TEXTURE_3D == texDesc.type)
			ToyGE_ASSERT(firstArray + arraySize <= static_cast<Texture*>(this)->Desc().depth);
		else
			ToyGE_ASSERT(firstArray + arraySize <= static_cast<Texture*>(this)->Desc().arraySize);
		ToyGE_ASSERT(RENDER_RESOURCE_TEXTURE == ResourceType());

		ResourceView view;
		view.resource = shared_from_this();
		view.subDesc.textureDesc.bAsCube = false;
		view.subDesc.textureDesc.firstMipLevel = firstMip;
		view.subDesc.textureDesc.mipLevels = mipLevels == 0 ? std::static_pointer_cast<Texture>(view.resource)->Desc().mipLevels : mipLevels;
		view.subDesc.textureDesc.firstArray = firstArray;
		view.subDesc.textureDesc.arraySize = arraySize == 0 ? std::static_pointer_cast<Texture>(view.resource)->Desc().arraySize : arraySize;
		view.subDesc.textureDesc.uavInitalCounts = uavInitalCounts;
		view.formatHint = formatHint;

		return view;
	}

	//texture view for cube
	ResourceView RenderResource::CreateTextureView_Cube(
		int32_t firstMip,
		int32_t mipLevels,
		int32_t firstFaceOffset,
		int32_t numCubes,
		RenderFormat formatHint,
		int32_t uavInitalCounts)
	{
		ToyGE_ASSERT(firstMip + mipLevels <= static_cast<Texture*>(this)->Desc().mipLevels);
		ToyGE_ASSERT(firstFaceOffset + numCubes * 6 <= static_cast<Texture*>(this)->Desc().arraySize * 6);
		ToyGE_ASSERT(RENDER_RESOURCE_TEXTURE == ResourceType());
		ToyGE_ASSERT(TEXTURE_CUBE == static_cast<Texture*>(this)->Desc().type);

		ResourceView view;
		view.resource = shared_from_this();
		view.subDesc.textureDesc.bAsCube = true;
		view.subDesc.textureDesc.firstMipLevel = firstMip;
		view.subDesc.textureDesc.mipLevels = mipLevels == 0 ? std::static_pointer_cast<Texture>(view.resource)->Desc().mipLevels : mipLevels;
		view.subDesc.textureDesc.firstFaceOffset = firstFaceOffset;
		view.subDesc.textureDesc.numCubes = numCubes;
		view.subDesc.textureDesc.uavInitalCounts = uavInitalCounts;
		view.formatHint = formatHint;

		return view;
	}

	void RenderResource::SetBind(ResourceBoundType bindState, int32_t index, ShaderType shaderType, RenderResourceSubDesc subDesc)
	{
		_bindShaderType = shaderType;
		_bindState = bindState;
		_boundIndex = index;
		_bindSubDesc = subDesc;
		_isBinded = true;
	}

	void RenderResource::SetBind(ResourceBoundType bindState, int32_t index, ShaderType shaderType)
	{
		_bindState = bindState;
		_boundIndex = index;
		_bindShaderType = shaderType;
		_isBinded = true;
	}

	void RenderResource::SetBind(ResourceBoundType bindState, int32_t index)
	{
		_bindState = bindState;
		_boundIndex = index;
		_isBinded = true;
	}

	void RenderResource::SetBind(ResourceBoundType bindState)
	{
		_bindState = bindState;
		_isBinded = true;
	}
}

