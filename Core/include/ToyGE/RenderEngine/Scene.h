#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Actor.h"
#include "ToyGE\RenderEngine\SceneCuller.h"

namespace ToyGE
{
	class RenderView;
	class RenderEffect;

	class TOYGE_CORE_API Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene();

		int32_t AddActor(const Ptr<Actor> & actor);

		Ptr<Actor> GetActor(int32_t actorID) const;

		void RemoveActor(const Ptr<Actor> & actor);

		void RemoveActor(int32_t objID);

		void AddView(const Ptr<RenderView> & view);

		int32_t NumViews() const
		{
			return static_cast<int32_t>(_views.size());
		}

		Ptr<RenderView> GetView(int32_t index) const
		{
			return _views[index];
		}

		CLASS_SET(AmbientColor, float3, _ambientColor);
		CLASS_GET(AmbientColor, float3, _ambientColor);

		CLASS_SET(AmbientTexture, Ptr<class Texture>, _ambientTex);
		CLASS_GET(AmbientTexture, Ptr<class Texture>, _ambientTex);

	private:
		static int32_t actorID;
		std::map<int32_t, Ptr<Actor>> _actorsMap;
		std::vector<Ptr<RenderView>> _views;
		float3 _ambientColor;
		Ptr<class Texture> _ambientTex;
	};
}

#endif