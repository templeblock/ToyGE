#pragma once
#ifndef AMBIENTHEMIPANORAMICMAP_H
#define AMBIENTHEMIPANORAMICMAP_H

#include "ToyGE\RenderEngine\Effects\AmbientMap.h"

namespace ToyGE
{
	DECLARE_SHADER(, AmbientHemiPanoramicMapVS, SHADER_VS, "AmbientMap", "AmbientHemiPanoramicMapVS", SM_4);
	DECLARE_SHADER(, AmbientHemiPanoramicMapPS, SHADER_PS, "AmbientMap", "AmbientHemiPanoramicMapPS", SM_4);

	class TOYGE_CORE_API AmbientHemiPanoramicMap : public AmbientMap
	{
	public:
		AmbientHemiPanoramicMap()
			: AmbientMap(AM_HEMIPANORAMIC)
		{

		}

		virtual void Render(const Ptr<RenderView> & view) override;
	};
}

#endif
