#pragma once
#ifndef REFLECTIONMAPCAPTURE_H
#define REFLECTIONMAPCAPTURE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class TOYGE_CORE_API ReflectionMapCapture
	{
	public:

		CLASS_SET(Pos, float3, _pos);
		CLASS_GET(Pos, float3, _pos);

		CLASS_SET(Radius, float, _radius);
		CLASS_GET(Radius, float, _radius);

		Ptr<class ReflectionMap> CaptureScene(const Ptr<class Scene> & scene);

	private:
		float3 _pos = 0.0f;
		float _radius = 100.0f;
	};
}

#endif
