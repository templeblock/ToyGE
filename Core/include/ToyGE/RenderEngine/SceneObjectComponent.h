#pragma once
#ifndef SCENEOBJECTCOMPONENT_H
#define SCENEOBJECTCOMPONENT_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	class SceneObject;

	class TOYGE_CORE_API SceneObjectComponent
	{
	public:
		SceneObjectComponent();

		virtual String GetComponentName() const = 0;

		void SetOwner(const Ptr<SceneObject> & owner);

		Ptr<SceneObject> GetOwner() const;

		bool IsActive() const
		{
			return _bActive;
		}

		void Active();

	protected:
		std::weak_ptr<SceneObject> _owner;
		bool _bActive;

		virtual void DoActive();
	};
}

#endif