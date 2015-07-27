#pragma once
#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class SceneObjectComponent;

	class TOYGE_CORE_API SceneObject : public std::enable_shared_from_this<SceneObject>
	{
	public:
		SceneObject();

		void AddComponent(const Ptr<SceneObjectComponent> & component);

		int32_t NumComponents() const
		{
			return static_cast<int32_t>(_components.size());
		}

		const Ptr<SceneObjectComponent> & GetComponent(int32_t index) const
		{
			return _components[index];
		}

		void RemoveComponent(const Ptr<SceneObjectComponent> & component);

		void ActiveAllComponents();

	private:
		std::vector<Ptr<SceneObjectComponent>> _components;
	};
}

#endif