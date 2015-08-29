#pragma once
#ifndef DEFERREDRENDERING_H
#define DEFERREDRENDERING_H

#include "ToyGE\RenderEngine\RenderAction.h"

namespace ToyGE
{
	class TOYGE_CORE_API DeferredRenderingBase : public RenderAction
	{
	public:
		DeferredRenderingBase();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<Texture> _normalsFittingTex;
		Ptr<RenderEffect> _fx;

		//void InitTextures(int32_t width, int32_t height);
	};

	class TOYGE_CORE_API DeferredRenderingLighting : public RenderAction
	{
	public:
		DeferredRenderingLighting();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _deferredFX;

		//void InitTextures(int32_t width, int32_t height);
	};

	class TOYGE_CORE_API DeferredRenderingShading : public RenderAction
	{
	public:
		DeferredRenderingShading();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _deferredFX;

		//void InitTextures(int32_t width, int32_t height);
	};
}

#endif