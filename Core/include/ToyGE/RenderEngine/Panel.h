#pragma once
#ifndef PANEL_H
#define PANEL_H

#include "ToyGE\RenderEngine\WidgetContainer.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"

namespace ToyGE
{
	class Texture;

	class TOYGE_CORE_API Panel : public WidgetContainer
	{
	public:
		Panel();

		virtual ~Panel() = default;

		virtual bool Intersect(const float2 & parentSpacePos) const override;

		CLASS_GET(BackgroundColor, float4, _backgroundColor);
		CLASS_SET(BackgroundColor, float4, _backgroundColor);

		CLASS_GET(BackgroundTexture, Ptr<Texture>, _backgroundTex);
		CLASS_SET(BackgroundTexture, Ptr<Texture>, _backgroundTex);

		CLASS_GET(Size, float2, _size);
		CLASS_SET(Size, float2, _size);

	protected:
		float4 _backgroundColor;
		Ptr<Texture> _backgroundTex;
		float2 _size;

		void RenderSelf(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;
	};
}

#endif