#include "ToyGE\RenderEngine\SceneObject.h"
#include "ToyGE\RenderEngine\SceneObjectComponent.h"

namespace ToyGE
{
	SceneObject::SceneObject()
	{
	}

	void SceneObject::AddComponent(const Ptr<SceneObjectComponent> & component)
	{
		component->SetOwner(shared_from_this());
		_components.push_back(component);
	}

	void SceneObject::RemoveComponent(const Ptr<SceneObjectComponent> & component)
	{
		if (component->GetOwner() == shared_from_this())
		{
			component->SetOwner(Ptr<SceneObject>());
			_components.erase(std::find(_components.begin(), _components.end(), component));
		}
	}

	void SceneObject::ActiveAllComponents()
	{
		for (auto & com : _components)
			com->Active();
	}
}