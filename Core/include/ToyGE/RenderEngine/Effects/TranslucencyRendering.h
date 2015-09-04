#pragma once
#ifndef TRANSLUCENTCYRENDERING_H
#define TRANSLUCENTCYRENDERING_H

#include "ToyGE\RenderEngine\RenderAction.h"
#include "ToyGE\RenderEngine\Effects\DeferredRendering.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

namespace ToyGE
{
	class Mesh;
	class LightComponent;
	class RenderBuffer;
	class Camera;
	class RenderComponent;

	class TOYGE_CORE_API TranslucencyRendering : public RenderAction
	{
	public:
		TranslucencyRendering();

		void Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment) override;

	private:
		Ptr<RenderEffect> _fx;
		Ptr<RenderEffect> _causticsFX;
		int32_t _causticsMapSize;
		std::map<Ptr<LightComponent>, Ptr<Texture>> _causticsRecieverPosMap;
		std::map<Ptr<LightComponent>, Ptr<Texture>> _causticsRenderMap;
		std::map<Ptr<Mesh>, Ptr<Mesh>> _casuticsMeshMap;
		Ptr<RenderBuffer> _oitCounterBuffer;
		Ptr<RenderEffect> _oitSortFX;

		void NoOITRender(
			const std::vector<Ptr<RenderComponent>> & objs,
			const std::vector<Ptr<LightComponent>> & lights,
			const Ptr<Camera> & camera,
			const Ptr<Texture> & targetTex, 
			const ResourceView & rawDepth);

		void OITRender(
			const std::vector<Ptr<RenderComponent>> & objs,
			const Ptr<LightComponent> & light,
			const Ptr<Camera> & camera,
			const ResourceView & target,
			const Ptr<Texture> & backgroundTex,
			const ResourceView & rawDepth);

		void RenderRefractionObject(
			const Ptr<RenderComponent> & obj,
			const std::vector<Ptr<LightComponent>> & lights,
			const Ptr<Camera> & camera,
			const ResourceView & target,
			const Ptr<Texture> & backgroundTex,
			const ResourceView & rawDepth,
			const Ptr<Texture> & linearDepth);

		void InitCausticsMapTextures(const Ptr<LightComponent> & light);

		Ptr<Mesh> CreateCausticsMesh(const Ptr<Mesh> & mesh);

		void InitCausticsMap(const Ptr<LightComponent> & light, float pointSize);
	};
}

#endif