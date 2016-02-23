#pragma once
#ifndef STATICMESHACTOR_H
#define STATICMESHACTOR_H

#include "ToyGE\RenderEngine\Actor.h"

namespace ToyGE
{
	class TOYGE_CORE_API StaticMeshActor : public Actor
	{
	public:
		static Ptr<StaticMeshActor> Create(const Ptr<class Scene> & scene, const Ptr<class Mesh> & mesh);

		static Ptr<StaticMeshActor> Create(const Ptr<class Scene> & scene, const String & meshPath);

		virtual ~StaticMeshActor() = default;

	protected:

	};
}

#endif
