#include "ToyGE\RenderEngine\SceneObjectComponent.h"

namespace ToyGE
{
	SceneObjectComponent::SceneObjectComponent()
		: _bActive(false)
	{
		
	}

	void SceneObjectComponent::SetOwner(const Ptr<SceneObject> & owner)
	{
		_owner = owner;
	}

	Ptr<SceneObject> SceneObjectComponent::GetOwner() const
	{
		return _owner.lock();
	}

	void SceneObjectComponent::Active()
	{
		DoActive();
		_bActive = true;
	}

	void SceneObjectComponent::DoActive()
	{
	}
}