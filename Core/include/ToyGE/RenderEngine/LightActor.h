#pragma once
#ifndef LIGHTACTOR_H
#define LIGHTACTOR_H

#include "ToyGE\RenderEngine\Actor.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	class TOYGE_CORE_API LightActor : public Actor
	{
	public:
		template <typename T>
		static Ptr<LightActor> Create(const Ptr<class Scene> & scene)
		{
			auto lightCom = std::make_shared<T>();
			auto lightActor = std::make_shared<LightActor>();
			lightActor->AddComponent(lightCom);
			scene->AddActor(lightActor);
			lightActor->ActivateAllComponents();
			return lightActor;
		}

		virtual ~LightActor() = default;

		template <typename T>
		Ptr<T> GetLight() const
		{
			return _rootTransformComponent->Cast<T>();
		}

	protected:

	};
}


#endif
