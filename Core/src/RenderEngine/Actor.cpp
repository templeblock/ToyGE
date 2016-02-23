#include "ToyGE\RenderEngine\Actor.h"
#include "ToyGE\RenderEngine\ActorComponent.h"
#include "ToyGE\RenderEngine\TransformComponent.h"

namespace ToyGE
{
	Actor::Actor()
	{
	}

	void Actor::Tick(float elapsedTime)
	{
		for (auto & com : _components)
			com->Tick(elapsedTime);
	}

	void Actor::AddComponent(const Ptr<ActorComponent> & component)
	{
		component->SetOwner(Cast<Actor>());
		_components.push_back(component);

		if (!_rootTransformComponent)
		{
			auto cast = component->DyCast<TransformComponent>();
			if (cast)
				_rootTransformComponent = cast;
		}
	}

	void Actor::RemoveComponent(const Ptr<ActorComponent> & component)
	{
		if (component->GetOwner().get() == this)
		{
			component->SetOwner(nullptr);
			_components.erase(std::find(_components.begin(), _components.end(), component));

			if (_rootTransformComponent == component)
				_rootTransformComponent = nullptr;
		}
	}

	void Actor::ActivateAllComponents()
	{
		for (auto & com : _components)
			com->Activate();
	}
}