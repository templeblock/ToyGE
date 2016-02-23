#pragma once
#ifndef ACTORCOMPONENT_H
#define ACTORCOMPONENT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"

namespace ToyGE
{
	class Actor;

	class TOYGE_CORE_API ActorComponent : public StaticCastable
	{
	public:
		ActorComponent()
		{
		}

		virtual ~ActorComponent() = default;

		virtual void Tick(float elapsedTime) {};

		void SetOwner(const Ptr<Actor> & owner)
		{
			_owner = owner;
		}

		Ptr<Actor> GetOwner() const
		{
			return _owner.lock();
		}

		bool IsActive() const
		{
			return _bActive;
		}
		virtual void Activate()
		{
			_bActive = true;
		}

	protected:
		std::weak_ptr<Actor> _owner;
		bool _bActive = false;
	};
}

#endif