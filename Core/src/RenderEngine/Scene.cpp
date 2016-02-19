#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Camera.h"
//#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	int32_t Scene::objID = 0;

	Scene::Scene()
		: _ambientColor(0.0f)
	{

	}

	int32_t Scene::AddSceneObject(const Ptr<SceneObject> & obj)
	{
		_sceneObjsMap[objID] = obj;
		return objID++;
	}

	Ptr<SceneObject> Scene::GetSceneObject(int32_t objID) const
	{
		auto objFind = _sceneObjsMap.find(objID);
		if (objFind != _sceneObjsMap.end())
			return objFind->second;
		else
			return Ptr<SceneObject>();
	}

	void Scene::RemoveSceneObject(const Ptr<SceneObject> & obj)
	{
		_sceneObjsMap.erase(std::find_if(_sceneObjsMap.begin(), _sceneObjsMap.end(),
			[&](const std::pair<int32_t, Ptr<SceneObject>> & e) -> bool
		{
			return e.second == obj;
		}));
	}

	void Scene::RemoveSceneObject(int32_t objID)
	{
		_sceneObjsMap.erase(objID);
	}

	void Scene::AddView(const Ptr<RenderView> & view)
	{
		_views.push_back(view);
		view->SetScene(shared_from_this());
	}

}