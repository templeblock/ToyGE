#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"

namespace ToyGE
{
	String CommonRenderShareName::GBuffer(int32_t index)
	{
		return String("GBuffer_") + std::to_string(index);
	}

	String CommonRenderShareName::Lighting(int32_t index)
	{
		return String("Lighting_") + std::to_string(index);
	}

	String CommonRenderShareName::RawDepth()
	{
		return "RawDepth";
	}

	String CommonRenderShareName::LinearDepth()
	{
		return "LinearDepth";
	}

	String CommonRenderShareName::Velocity()
	{
		return "Velocity";
	}


	void RenderSharedEnviroment::Clear()
	{
		for (auto & i : _sharedParams)
			i.second->Release();
		_sharedParams.clear();
	}

	Ptr<Texture> RenderSharedEnviroment::GetTextureParam(const String & name) const
	{
		return ParamByName(name)->As<SharedParam<Ptr<Texture>>>()->GetValue();
	}
}