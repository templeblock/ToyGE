#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Camera.h"

namespace ToyGE
{
	int32_t Scene::actorID = 0;

	Scene::Scene()
		: _ambientColor(0.0f)
	{

	}

	int32_t Scene::AddActor(const Ptr<Actor> & actor)
	{
		_actorsMap[actorID] = actor;
		return actorID++;
	}

	Ptr<Actor> Scene::GetActor(int32_t objID) const
	{
		auto objFind = _actorsMap.find(objID);
		if (objFind != _actorsMap.end())
			return objFind->second;
		else
			return nullptr;
	}

	void Scene::RemoveActor(const Ptr<Actor> & obj)
	{
		_actorsMap.erase(std::find_if(_actorsMap.begin(), _actorsMap.end(),
			[&](const std::pair<int32_t, Ptr<Actor>> & e) -> bool
		{
			return e.second == obj;
		}));
	}

	void Scene::RemoveActor(int32_t objID)
	{
		_actorsMap.erase(objID);
	}

	void Scene::AddView(const Ptr<RenderView> & view)
	{
		_views.push_back(view);
		view->SetScene(shared_from_this());
	}

}