#pragma once
#ifndef SCENECULLER_H
#define SCENECULLER_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class TOYGE_CORE_API Cullable
	{
	public:
		bool cullState = false;
		int32_t cullerHandle = 0;

		virtual AABBox GetBoundsAABB() const
		{
			return AABBox();
		}

		virtual OBBox GetBoundsOBB() const
		{
			return OBBox();
		}

		virtual Sphere GetBoundsSphere() const
		{
			return Sphere();
		}

		virtual Frustum GetBoundsFrustum() const
		{
			return Frustum();
		}

	};

	class TOYGE_CORE_API SceneCuller
	{
	public:
		virtual ~SceneCuller() = default;

		virtual AABBox GetSceneAABB() = 0;

		virtual void AddElement(const Ptr<Cullable> & element) = 0;

		virtual void RemoveElement(const Ptr<Cullable> & emelent) = 0;

		virtual void UpdateElement(const Ptr<Cullable> & element) = 0;

		virtual void GetAllElements(std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const AABBox & aabb, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const OBBox & obb, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const Frustum & frustum, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const Sphere & sphere, std::vector<Ptr<Cullable>> & outElements) = 0;
	};
}

#endif