#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class SceneObject;
	class RenderView;
	class RenderEffect;

	class TOYGE_CORE_API Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene();

		int32_t AddSceneObject(const Ptr<SceneObject> & obj);

		Ptr<SceneObject> GetSceneObject(int32_t objID) const;

		void RemoveSceneObject(const Ptr<SceneObject> & obj);

		void RemoveSceneObject(int32_t objID);

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

		void BindParams(const Ptr<RenderEffect> & fx);

	private:
		static int32_t objID;
		std::map<int32_t, Ptr<SceneObject>> _sceneObjsMap;
		std::vector<Ptr<RenderView>> _views;
		float3 _ambientColor;
	};
}

#endif