#pragma once
#ifndef AMBIENTCUBEMAP_H
#define AMBIENTCUBEMAP_H

#include "ToyGE\RenderEngine\Effects\AmbientMap.h"

namespace ToyGE
{
	DECLARE_SHADER(, AmbientCubeMapVS, SHADER_VS, "AmbientMap", "AmbientCubeMapVS", SM_4);
	DECLARE_SHADER(, AmbientCubeMapPS, SHADER_PS, "AmbientMap", "AmbientCubeMapPS", SM_4);

	class TOYGE_CORE_API AmbientCubeMap : public AmbientMap
	{
	public:
		AmbientCubeMap()
			: AmbientMap(AM_CUBEMAP)
		{

		}

		virtual void Render(const Ptr<class RenderView> & view) override;
	};
}

#endif
