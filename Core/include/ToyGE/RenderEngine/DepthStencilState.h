#pragma once
#ifndef DEPTHSTENCILSHADERATE_H
#define DEPTHSTENCILSHADERATE_H

#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	enum StencilOperation : uint32_t
	{
		STENCIL_OP_KEEP		= 0,
		STENCIL_OP_ZERO		= 1,
		STENCIL_OP_REPLACE	= 2,
		STENCIL_OP_INCR_SAT = 3,
		STENCIL_OP_DECR_SAT = 4,
		STENCIL_OP_INVERT	= 5,
		STENCIL_OP_INCR		= 6,
		STENCIL_OP_DECR		= 7
	};

	enum DepthWriteMask : uint32_t
	{
		DEPTH_WRITE_ZERO	= 0,
		DEPTH_WRITE_ALL		= 1
	};

	struct StencilOperationDesc
	{
		StencilOperation    stencilFailOp		= STENCIL_OP_KEEP;
		StencilOperation    stencilDepthFailOp	= STENCIL_OP_KEEP;
		StencilOperation    stencilPassOp		= STENCIL_OP_KEEP;
		ComparisonFunc		stencilFunc			= COMPARISON_ALWAYS;
	};

	struct DepthStencilStateDesc
	{
		bool					depthEnable			= true;
		DepthWriteMask			depthWriteMask		= DEPTH_WRITE_ALL;
		ComparisonFunc			depthFunc			= COMPARISON_LESS;
		bool					stencilEnable		= false;
		uint8_t					stencilReadMask		= 0xff;
		uint8_t					stencilWriteMask	= 0xff;
		StencilOperationDesc	frontFace;
		StencilOperationDesc	backFace;
	};

	class TOYGE_CORE_API DepthStencilState : public RenderResource
	{
	public:
		DepthStencilState() : RenderResource(RenderResourceType::RRT_DEPTHSTENCILSTATE) {};

		virtual ~DepthStencilState() = default;

		CLASS_SET(Desc, DepthStencilStateDesc, _desc);
		CLASS_GET(Desc, DepthStencilStateDesc, _desc);

	protected:
		DepthStencilStateDesc _desc;
	};

	template <
		bool				depthEnable = true,
		DepthWriteMask		depthWriteMask = DEPTH_WRITE_ALL,
		ComparisonFunc		depthFunc = COMPARISON_LESS,
		bool				stencilEnable = false,
		uint8_t				stencilReadMask = 0xff,
		uint8_t				stencilWriteMask = 0xff,

		StencilOperation    frontFaceStencilFailOp = STENCIL_OP_KEEP,
		StencilOperation    frontFaceStencilDepthFailOp = STENCIL_OP_KEEP,
		StencilOperation    frontFaceStencilPassOp = STENCIL_OP_KEEP,
		ComparisonFunc		frontFaceStencilFunc = COMPARISON_ALWAYS,

		StencilOperation    backFaceStencilFailOp = STENCIL_OP_KEEP,
		StencilOperation    backFaceStencilDepthFailOp = STENCIL_OP_KEEP,
		StencilOperation    backFaceStencilPassOp = STENCIL_OP_KEEP,
		ComparisonFunc		backFaceStencilFunc = COMPARISON_ALWAYS
	>
	class TOYGE_CORE_API DepthStencilStateTemplate
	{
	public:
		static const Ptr<DepthStencilState> & Get()
		{
			if (!_state)
			{
				_state = Global::GetRenderEngine()->GetRenderFactory()->CreateDepthStencilState();

				DepthStencilStateDesc desc;

				desc.depthEnable = depthEnable;
				desc.depthWriteMask = depthWriteMask;
				desc.depthFunc = depthFunc;
				desc.stencilEnable = stencilEnable;
				desc.stencilReadMask = stencilReadMask;
				desc.stencilWriteMask = stencilWriteMask;

				desc.frontFace.stencilFailOp = frontFaceStencilFailOp;
				desc.frontFace.stencilDepthFailOp = frontFaceStencilDepthFailOp;
				desc.frontFace.stencilPassOp = frontFaceStencilPassOp;
				desc.frontFace.stencilFunc = frontFaceStencilFunc;

				desc.backFace.stencilFailOp = backFaceStencilFailOp;
				desc.backFace.stencilDepthFailOp = backFaceStencilDepthFailOp;
				desc.backFace.stencilPassOp = backFaceStencilPassOp;
				desc.backFace.stencilFunc = backFaceStencilFunc;

				_state->SetDesc(desc);
				_state->Init();
			}

			return _state;
		}

	private:
		static Ptr<DepthStencilState> _state;
	};

#ifdef TOYGE_CORE_SOURCE
	template <
		bool				depthEnable,
		DepthWriteMask		depthWriteMask,
		ComparisonFunc		depthFunc,
		bool				stencilEnable,
		uint8_t				stencilReadMask,
		uint8_t				stencilWriteMask,

		StencilOperation    frontFaceStencilFailOp,
		StencilOperation    frontFaceStencilDepthFailOp,
		StencilOperation    frontFaceStencilPassOp,
		ComparisonFunc		frontFaceStencilFunc,

		StencilOperation    backFaceStencilFailOp,
		StencilOperation    backFaceStencilDepthFailOp,
		StencilOperation    backFaceStencilPassOp,
		ComparisonFunc		backFaceStencilFunc
	>
		Ptr<DepthStencilState> DepthStencilStateTemplate
		<
		depthEnable,
		depthWriteMask,
		depthFunc,
		stencilEnable,
		stencilReadMask,
		stencilWriteMask,

		frontFaceStencilFailOp,
		frontFaceStencilDepthFailOp,
		frontFaceStencilPassOp,
		frontFaceStencilFunc,

		backFaceStencilFailOp,
		backFaceStencilDepthFailOp,
		backFaceStencilPassOp,
		backFaceStencilFunc
		>::_state;
#endif
}

#endif