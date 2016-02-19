#pragma once
#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	enum BlendParam : uint32_t
	{
		BLEND_PARAM_ZERO = 1,
		BLEND_PARAM_ONE = 2,
		BLEND_PARAM_SRC_COLOR = 3,
		BLEND_PARAM_INV_SRC_COLOR = 4,
		BLEND_PARAM_SRC_ALPHA = 5,
		BLEND_PARAM_INV_SRC_ALPHA = 6,
		BLEND_PARAM_DEST_ALPHA = 7,
		BLEND_PARAM_INV_DEST_ALPHA = 8,
		BLEND_PARAM_DEST_COLOR = 9,
		BLEND_PARAM_INV_DEST_COLOR = 10,
		BLEND_PARAM_SRC_ALPHA_SAT = 11,
		BLEND_PARAM_BLEND_FACTOR = 14,
		BLEND_PARAM_INV_BLEND_FACTOR = 15,
		BLEND_PARAM_SRC1_COLOR = 16,
		BLEND_PARAM_INV_SRC1_COLOR = 17,
		BLEND_PARAM_SRC1_ALPHA = 18,
		BLEND_PARAM_INV_SRC1_ALPHA = 19
	};

	enum BlendOperation : uint32_t
	{
		BLEND_OP_ADD = 1,
		BLEND_OP_SUBTRACT = 2,
		BLEND_OP_REV_SUBTRACT = 3,
		BLEND_OP_MIN = 4,
		BLEND_OP_MAX = 5
	};

	struct BlendRenderTargetDesc
	{
		bool     blendEnable = false;
		BlendParam    srcBlend = BLEND_PARAM_ONE;
		BlendParam    dstBlend = BLEND_PARAM_ZERO;
		BlendOperation blendOP = BLEND_OP_ADD;
		BlendParam    srcBlendAlpha = BLEND_PARAM_ONE;
		BlendParam    dstBlendAlpha = BLEND_PARAM_ONE;
		BlendOperation blendOPAlpha = BLEND_OP_ADD;
		uint8_t  renderTargetWriteMask = COLOR_WRITE_ALL;
	};

	struct BlendStateDesc
	{
		bool			alphaToCoverageEnable = false;
		bool			independentBlendEnable = false;
		BlendRenderTargetDesc	blendRTDesc[8];
	};

	class TOYGE_CORE_API BlendState : public RenderResource
	{
	public:
		BlendState() : RenderResource(RenderResourceType::RRT_BLENDSTATE) {};

		virtual ~BlendState() = default;

		CLASS_SET(Desc, BlendStateDesc, _desc);
		CLASS_GET(Desc, BlendStateDesc, _desc);

	protected:
		BlendStateDesc _desc;
	};


	template <
		bool alphaToCoverageEnable = false,
		bool independentBlendEnable = false,

		bool			rt0_BlendEnable = false,
		BlendParam		rt0_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt0_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt0_BlendOP = BLEND_OP_ADD,
		BlendParam		rt0_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt0_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt0_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt0_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt1_BlendEnable = false,
		BlendParam		rt1_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt1_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt1_BlendOP = BLEND_OP_ADD,
		BlendParam		rt1_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt1_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt1_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt1_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt2_BlendEnable = false,
		BlendParam		rt2_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt2_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt2_BlendOP = BLEND_OP_ADD,
		BlendParam		rt2_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt2_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt2_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt2_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt3_BlendEnable = false,
		BlendParam		rt3_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt3_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt3_BlendOP = BLEND_OP_ADD,
		BlendParam		rt3_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt3_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt3_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt3_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt4_BlendEnable = false,
		BlendParam		rt4_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt4_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt4_BlendOP = BLEND_OP_ADD,
		BlendParam		rt4_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt4_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt4_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt4_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt5_BlendEnable = false,
		BlendParam		rt5_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt5_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt5_BlendOP = BLEND_OP_ADD,
		BlendParam		rt5_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt5_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt5_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt5_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt6_BlendEnable = false,
		BlendParam		rt6_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt6_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt6_BlendOP = BLEND_OP_ADD,
		BlendParam		rt6_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt6_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt6_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt6_RenderTargetWriteMask = COLOR_WRITE_ALL,

		bool			rt7_BlendEnable = false,
		BlendParam		rt7_SrcBlend = BLEND_PARAM_ONE,
		BlendParam		rt7_DstBlend = BLEND_PARAM_ZERO,
		BlendOperation	rt7_BlendOP = BLEND_OP_ADD,
		BlendParam		rt7_SrcBlendAlpha = BLEND_PARAM_ONE,
		BlendParam		rt7_DstBlendAlpha = BLEND_PARAM_ONE,
		BlendOperation	rt7_BlendOPAlpha = BLEND_OP_ADD,
		uint8_t			rt7_RenderTargetWriteMask = COLOR_WRITE_ALL
	>
	class TOYGE_CORE_API BlendStateTemplate
	{
	public:
		static const Ptr<BlendState> & Get()
		{
			if (!_state)
			{
				_state = Global::GetRenderEngine()->GetRenderFactory()->CreateBlendState();
				BlendStateDesc desc;
				desc.alphaToCoverageEnable = alphaToCoverageEnable;
				desc.independentBlendEnable = independentBlendEnable;

				#define BLEND_RT_DESC(i) \
				desc.blendRTDesc[i].blendEnable				= rt ## i ## _BlendEnable;\
				desc.blendRTDesc[i].srcBlend				= rt ## i ## _SrcBlend;\
				desc.blendRTDesc[i].dstBlend				= rt ## i ## _DstBlend;\
				desc.blendRTDesc[i].blendOP					= rt ## i ## _BlendOP;\
				desc.blendRTDesc[i].srcBlendAlpha			= rt ## i ## _SrcBlendAlpha;\
				desc.blendRTDesc[i].dstBlendAlpha			= rt ## i ## _DstBlendAlpha;\
				desc.blendRTDesc[i].blendOPAlpha			= rt ## i ## _BlendOPAlpha;\
				desc.blendRTDesc[i].renderTargetWriteMask	= rt ## i ## _RenderTargetWriteMask;

				BLEND_RT_DESC(0);
				BLEND_RT_DESC(1);
				BLEND_RT_DESC(2);
				BLEND_RT_DESC(3);
				BLEND_RT_DESC(4);
				BLEND_RT_DESC(5);
				BLEND_RT_DESC(6);
				BLEND_RT_DESC(7);

				#undef BLEND_RT_DESC

				_state->SetDesc(desc);
				_state->Init();
			}

			return _state;
		}

	private:
		static Ptr<BlendState> _state;
	};

#ifdef TOYGE_CORE_SOURCE
	template <
		bool alphaToCoverageEnable,
		bool independentBlendEnable,

		bool			rt0_BlendEnable,
		BlendParam		rt0_SrcBlend,
		BlendParam		rt0_DstBlend,
		BlendOperation	rt0_BlendOP,
		BlendParam		rt0_SrcBlendAlpha,
		BlendParam		rt0_DstBlendAlpha,
		BlendOperation	rt0_BlendOPAlpha,
		uint8_t			rt0_RenderTargetWriteMask,

		bool			rt1_BlendEnable,
		BlendParam		rt1_SrcBlend,
		BlendParam		rt1_DstBlend,
		BlendOperation	rt1_BlendOP,
		BlendParam		rt1_SrcBlendAlpha,
		BlendParam		rt1_DstBlendAlpha,
		BlendOperation	rt1_BlendOPAlpha,
		uint8_t			rt1_RenderTargetWriteMask,

		bool			rt2_BlendEnable,
		BlendParam		rt2_SrcBlend,
		BlendParam		rt2_DstBlend,
		BlendOperation	rt2_BlendOP,
		BlendParam		rt2_SrcBlendAlpha,
		BlendParam		rt2_DstBlendAlpha,
		BlendOperation	rt2_BlendOPAlpha,
		uint8_t			rt2_RenderTargetWriteMask,

		bool			rt3_BlendEnable,
		BlendParam		rt3_SrcBlend,
		BlendParam		rt3_DstBlend,
		BlendOperation	rt3_BlendOP,
		BlendParam		rt3_SrcBlendAlpha,
		BlendParam		rt3_DstBlendAlpha,
		BlendOperation	rt3_BlendOPAlpha,
		uint8_t			rt3_RenderTargetWriteMask,

		bool			rt4_BlendEnable,
		BlendParam		rt4_SrcBlend,
		BlendParam		rt4_DstBlend,
		BlendOperation	rt4_BlendOP,
		BlendParam		rt4_SrcBlendAlpha,
		BlendParam		rt4_DstBlendAlpha,
		BlendOperation	rt4_BlendOPAlpha,
		uint8_t			rt4_RenderTargetWriteMask,

		bool			rt5_BlendEnable,
		BlendParam		rt5_SrcBlend,
		BlendParam		rt5_DstBlend,
		BlendOperation	rt5_BlendOP,
		BlendParam		rt5_SrcBlendAlpha,
		BlendParam		rt5_DstBlendAlpha,
		BlendOperation	rt5_BlendOPAlpha,
		uint8_t			rt5_RenderTargetWriteMask,

		bool			rt6_BlendEnable,
		BlendParam		rt6_SrcBlend,
		BlendParam		rt6_DstBlend,
		BlendOperation	rt6_BlendOP,
		BlendParam		rt6_SrcBlendAlpha,
		BlendParam		rt6_DstBlendAlpha,
		BlendOperation	rt6_BlendOPAlpha,
		uint8_t			rt6_RenderTargetWriteMask,

		bool			rt7_BlendEnable,
		BlendParam		rt7_SrcBlend,
		BlendParam		rt7_DstBlend,
		BlendOperation	rt7_BlendOP,
		BlendParam		rt7_SrcBlendAlpha,
		BlendParam		rt7_DstBlendAlpha,
		BlendOperation	rt7_BlendOPAlpha,
		uint8_t			rt7_RenderTargetWriteMask
	>
		Ptr<BlendState> BlendStateTemplate
		<
		alphaToCoverageEnable,
		independentBlendEnable,

		rt0_BlendEnable,
		rt0_SrcBlend,
		rt0_DstBlend,
		rt0_BlendOP,
		rt0_SrcBlendAlpha,
		rt0_DstBlendAlpha,
		rt0_BlendOPAlpha,
		rt0_RenderTargetWriteMask,

		rt1_BlendEnable,
		rt1_SrcBlend,
		rt1_DstBlend,
		rt1_BlendOP,
		rt1_SrcBlendAlpha,
		rt1_DstBlendAlpha,
		rt1_BlendOPAlpha,
		rt1_RenderTargetWriteMask,

		rt2_BlendEnable,
		rt2_SrcBlend,
		rt2_DstBlend,
		rt2_BlendOP,
		rt2_SrcBlendAlpha,
		rt2_DstBlendAlpha,
		rt2_BlendOPAlpha,
		rt2_RenderTargetWriteMask,

		rt3_BlendEnable,
		rt3_SrcBlend,
		rt3_DstBlend,
		rt3_BlendOP,
		rt3_SrcBlendAlpha,
		rt3_DstBlendAlpha,
		rt3_BlendOPAlpha,
		rt3_RenderTargetWriteMask,

		rt4_BlendEnable,
		rt4_SrcBlend,
		rt4_DstBlend,
		rt4_BlendOP,
		rt4_SrcBlendAlpha,
		rt4_DstBlendAlpha,
		rt4_BlendOPAlpha,
		rt4_RenderTargetWriteMask,

		rt5_BlendEnable,
		rt5_SrcBlend,
		rt5_DstBlend,
		rt5_BlendOP,
		rt5_SrcBlendAlpha,
		rt5_DstBlendAlpha,
		rt5_BlendOPAlpha,
		rt5_RenderTargetWriteMask,

		rt6_BlendEnable,
		rt6_SrcBlend,
		rt6_DstBlend,
		rt6_BlendOP,
		rt6_SrcBlendAlpha,
		rt6_DstBlendAlpha,
		rt6_BlendOPAlpha,
		rt6_RenderTargetWriteMask,

		rt7_BlendEnable,
		rt7_SrcBlend,
		rt7_DstBlend,
		rt7_BlendOP,
		rt7_SrcBlendAlpha,
		rt7_DstBlendAlpha,
		rt7_BlendOPAlpha,
		rt7_RenderTargetWriteMask
		> ::_state;
#endif

}

#endif