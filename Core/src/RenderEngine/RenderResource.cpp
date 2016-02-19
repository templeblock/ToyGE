#include "ToyGE\RenderEngine\RenderResource.h"

namespace ToyGE
{
	//std::list<std::weak_ptr<RenderResource>> RenderResource::_renderResources;

	RenderResource::RenderResource(RenderResourceType type)
		: _resourceType(type)
	{
	}

	RenderResource::~RenderResource()
	{
		Release();
	}

	void RenderResource::Init()
	{
		if (_bInit)
			Release();
		_bInit = true;
	}

	void RenderResource::Release()
	{
		_bInit = false;
	}

	void RenderResource::SetBound(RenderResourceBoundType boundType, ShaderType boundShaderType, int32_t boundIndex)
	{
		_bBound = true;

		_boundState.boundType = boundType;
		_boundState.boundShaderType = boundShaderType;
		_boundState.boundIndex = boundIndex;
	}

	void RenderResource::SetBound(const RenderResourceBoundState & boundState)
	{
		_bBound = true;

		_boundState = boundState;
	}
}

