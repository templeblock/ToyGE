#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class SceneObject;
	class RenderView;

	class TOYGE_CORE_API Scene
	{
	public:
		int32_t AddSceneObject(const Ptr<SceneObject> & obj);

		Ptr<SceneObject> GetSceneObject(int32_t objID) const;

		void RemoveSceneObject(const Ptr<SceneObject> & obj);

		void RemoveSceneObject(int32_t objID);

		void AddView(const Ptr<RenderView> & view)
		{
			_views.push_back(view);
		}

		int32_t NumViews() const
		{
			return static_cast<int32_t>(_views.size());
		}

		Ptr<RenderView> GetView(int32_t index) const
		{
			return _views[index];
		}

		//void InitViews();

		/*void RenderSpecific(uint32_t stepID);
		void RenderExtra();*/

	private:
		static int32_t objID;
		std::map<int32_t, Ptr<SceneObject>> _sceneObjsMap;
		std::vector<Ptr<RenderView>> _views;
	};
}

#endif