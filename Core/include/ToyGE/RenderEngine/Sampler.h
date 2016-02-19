#pragma once
#ifndef SAMPLER_H
#define SAMPLER_H

#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

namespace ToyGE
{
	enum Filter : uint32_t
	{
		FILTER_MIN_MAG_MIP_POINT = 0,
		FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
		FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
		FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
		FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
		FILTER_MIN_MAG_MIP_LINEAR = 0x15,
		FILTER_ANISOTROPIC = 0x55,
		FILTER_COMPARISON_MIN_MAG_MIP_POINT = 0x80,
		FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
		FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
		FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
		FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
		FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
		FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
		FILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
		FILTER_COMPARISON_ANISOTROPIC = 0xd5,
		FILTER_MINIMUM_MIN_MAG_MIP_POINT = 0x100,
		FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
		FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
		FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
		FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
		FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
		FILTER_MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
		FILTER_MINIMUM_ANISOTROPIC = 0x155,
		FILTER_MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
		FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
		FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
		FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
		FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
		FILTER_MAXIMUM_ANISOTROPIC = 0x1d5
	};

	enum TextureAddressMode : uint32_t
	{
		TEXTURE_ADDRESS_WRAP = 1,
		TEXTURE_ADDRESS_MIRROR = 2,
		TEXTURE_ADDRESS_CLAMP = 3,
		TEXTURE_ADDRESS_BORDER = 4,
		TEXTURE_ADDRESS_MIRROR_ONCE = 5
	};

	struct SamplerDesc
	{
		Filter					filter = FILTER_MIN_MAG_MIP_LINEAR;
		TextureAddressMode	addressU = TEXTURE_ADDRESS_CLAMP;
		TextureAddressMode	addressV = TEXTURE_ADDRESS_CLAMP;
		TextureAddressMode	addressW = TEXTURE_ADDRESS_CLAMP;
		float                   mipLODBias = 0.0f;
		uint8_t                 maxAnisotropy = 0;
		ComparisonFunc			comparisonFunc = COMPARISON_NEVER;
		float4                   borderColor = 0.0f;
		float                   minLOD = -FLT_MAX;
		float                   maxLOD = FLT_MAX;
	};

	class TOYGE_CORE_API Sampler : public RenderResource
	{
	public:
		Sampler() : RenderResource(RenderResourceType::RRT_SAMPLER) {};

		virtual ~Sampler() = default;

		CLASS_SET(Desc, SamplerDesc, _desc);
		CLASS_GET(Desc, SamplerDesc, _desc);

	protected:
		SamplerDesc _desc;
	};

	template <
		Filter				filter = FILTER_MIN_MAG_MIP_LINEAR,
		TextureAddressMode	addressU = TEXTURE_ADDRESS_CLAMP,
		TextureAddressMode	addressV = TEXTURE_ADDRESS_CLAMP,
		TextureAddressMode	addressW = TEXTURE_ADDRESS_CLAMP,
		uint8_t             maxAnisotropy = 0,
		ComparisonFunc		comparisonFunc = COMPARISON_NEVER
	>
	class TOYGE_CORE_API SamplerTemplate
	{
	public:
		static const Ptr<Sampler> & Get()
		{
			if (!_sampler)
			{
				_sampler = Global::GetRenderEngine()->GetRenderFactory()->CreateSampler();

				SamplerDesc desc;
				desc.filter = filter;
				desc.addressU = addressU;
				desc.addressV = addressV;
				desc.addressW = addressW;
				desc.maxAnisotropy = maxAnisotropy;
				desc.comparisonFunc = comparisonFunc;

				_sampler->SetDesc(desc);
				_sampler->Init();
			}
			return _sampler;
		}

	private:
		static Ptr<Sampler> _sampler;
	};

#ifdef TOYGE_CORE_SOURCE
	template <
		Filter				filter,
		TextureAddressMode	addressU,
		TextureAddressMode	addressV,
		TextureAddressMode	addressW,
		uint8_t             maxAnisotropy,
		ComparisonFunc		comparisonFunc
	>
		Ptr<Sampler> SamplerTemplate
		<
		filter,
		addressU,
		addressV,
		addressW,
		maxAnisotropy,
		comparisonFunc
		>::_sampler;
#endif
}

#endif