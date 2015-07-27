#pragma once
#ifndef SAMPLER_H
#define SAMPLER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class TOYGE_CORE_API Sampler
	{
	public:
		Sampler(const SamplerDesc & desc);

		virtual ~Sampler() = default;

		const SamplerDesc & Desc() const
		{
			return _desc;
		}

	private:
		SamplerDesc _desc;
	};
}

#endif