#pragma once
#ifndef OCEANACTOR_H
#define OCEANACTOR_H

#include "ToyGE\RenderEngine\Actor.h"
#include "ToyGE\RenderEngine\OceanRenderComponent.h"

namespace ToyGE
{
	class TOYGE_CORE_API OceanActor : public Actor
	{
	public:
		static Ptr<OceanActor> Create()
		{
			auto actor = std::make_shared<OceanActor>();
			actor->AddComponent(std::make_shared<OceanRenderComponent>());
			return actor;
		}

		OceanActor();

	private:

	};
}

#endif
