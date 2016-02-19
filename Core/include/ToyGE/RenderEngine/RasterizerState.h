#pragma once
#ifndef RASTERIZERSHADERATE_H
#define RASTERIZERSHADERATE_H

#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	enum FillMode : uint32_t
	{
		FILL_WIREFRAME	= 0,
		FILL_SOLID		= 1
	};

	enum CullMode : uint32_t
	{
		CULL_NONE	= 0,
		CULL_FRONT	= 1,
		CULL_BACK	= 2
	};

	struct RasterizerStateDesc
	{
		FillMode fillMode = FILL_SOLID;
		CullMode cullMode = CULL_BACK;
		bool      bFrontCounterClockwise = false;
		int32_t       depthBias = 0;
		float     depthBiasClamp = 0.0f;
		float     slopeScaledDepthBias = 0.0f;
		bool      depthClipEnable = true;
		bool      scissorEnable = false;
		bool      multisampleEnable = false;
		bool      antialiasedLineEnable = false;
	};

	class TOYGE_CORE_API RasterizerState : public RenderResource
	{
	public:
		RasterizerState() : RenderResource(RenderResourceType::RRT_RASTERIZERSTATE) {};

		virtual ~RasterizerState() = default;

		CLASS_SET(Desc, RasterizerStateDesc, _desc);
		CLASS_GET(Desc, RasterizerStateDesc, _desc);

	protected:
		RasterizerStateDesc _desc;
	};


	template <
		FillMode  fillMode = FILL_SOLID,
		CullMode  cullMode = CULL_BACK,
		bool      bFrontCounterClockwise = false,
		int32_t   depthBias = 0,
		bool      depthClipEnable = true,
		bool      scissorEnable = false,
		bool      multisampleEnable = false,
		bool      antialiasedLineEnable = false
	>
	class TOYGE_CORE_API RasterizerStateTemplate
	{
	public:
		static const Ptr<RasterizerState> & Get()
		{
			if (!_state)
			{
				_state = Global::GetRenderEngine()->GetRenderFactory()->CreateRasterizerState();

				RasterizerStateDesc desc;
				desc.fillMode = fillMode;
				desc.cullMode = cullMode;
				desc.bFrontCounterClockwise = bFrontCounterClockwise;
				desc.depthBias = depthBias;
				desc.depthClipEnable = depthClipEnable;
				desc.scissorEnable = scissorEnable;
				desc.multisampleEnable = multisampleEnable;
				desc.antialiasedLineEnable = antialiasedLineEnable;

				_state->SetDesc(desc);
				_state->Init();
			}
			return _state;
		}

	private:
		static Ptr<RasterizerState> _state;
	};

#ifdef TOYGE_CORE_SOURCE
	template <
		FillMode  fillMode,
		CullMode  cullMode,
		bool      bFrontCounterClockwise,
		int32_t   depthBias,
		bool      depthClipEnable,
		bool      scissorEnable,
		bool      multisampleEnable,
		bool      antialiasedLineEnable
	>
		Ptr<RasterizerState> RasterizerStateTemplate
		<
		fillMode,
		cullMode,
		bFrontCounterClockwise,
		depthBias,
		depthClipEnable,
		scissorEnable,
		multisampleEnable,
		antialiasedLineEnable
		>::_state;
#endif
}

#endif