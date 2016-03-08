#pragma once
#ifndef AMBIENTPANORAMICMAP_H
#define AMBIENTPANORAMICMAP_H

#include "ToyGE\RenderEngine\Effects\AmbientMap.h"

namespace ToyGE
{
	DECLARE_SHADER(, AmbientPanoramicMapVS, SHADER_VS, "AmbientMap", "AmbientPanoramicMapVS", SM_4);
	DECLARE_SHADER(, AmbientPanoramicMapPS, SHADER_PS, "AmbientMap", "AmbientPanoramicMapPS", SM_4);

	class TOYGE_CORE_API AmbientPanoramicMap : public AmbientMap
	{
	public:
		AmbientPanoramicMap()
			: AmbientMap(AM_PANORAMIC)
		{
		}

		virtual void Render(const Ptr<RenderView> & view) override;
	};
}

#endif
