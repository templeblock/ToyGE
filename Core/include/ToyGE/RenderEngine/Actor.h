#pragma once
#ifndef ACTOR_H
#define ACTOR_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"

namespace ToyGE
{
	class ActorComponent;

	class TOYGE_CORE_API Actor : public StaticCastable
	{
	public:
		Actor();

		virtual ~Actor() = default;

		virtual void Tick(float elapsedTime);

		void AddComponent(const Ptr<ActorComponent> & component);

		int32_t NumComponents() const
		{
			return static_cast<int32_t>(_components.size());
		}

		const Ptr<ActorComponent> & GetComponent(int32_t index) const
		{
			return _components[index];
		}

		template<typename T>
		Ptr<ActorComponent> FindComponent() const
		{
			for (auto & com : _components)
			{
				auto cast = std::dynamic_pointer_cast<T>(com);
				if (cast)
					return cast;
			}
			return nullptr;
		}

		void RemoveComponent(const Ptr<ActorComponent> & component);

		void SetRootTransformComponent(const Ptr<class TransformComponent> & transform)
		{
			_rootTransformComponent = transform;
		}
		Ptr<class TransformComponent> GetRootTransformComponent() const
		{
			return _rootTransformComponent;
		}

		void ActivateAllComponents();

	protected:
		Ptr<class TransformComponent> _rootTransformComponent;
		std::vector<Ptr<ActorComponent>> _components;
	};
}

#endif