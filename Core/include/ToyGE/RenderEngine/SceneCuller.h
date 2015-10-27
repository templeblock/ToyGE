#pragma once
#ifndef SCENECULLER_H
#define SCENECULLER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class TOYGE_CORE_API Cullable
	{
	public:
		virtual XNA::AxisAlignedBox GetBoundsAABB() const
		{
			XNA::AxisAlignedBox aabb;
			memset(&aabb, 0, sizeof(aabb));
			return aabb;
		}

		virtual XNA::OrientedBox GetBoundsOBB() const
		{
			XNA::OrientedBox obb;
			memset(&obb, 0, sizeof(obb));
			return obb;
		}

		virtual XNA::Sphere GetBoundsSphere() const
		{
			XNA::Sphere sphere;
			memset(&sphere, 0, sizeof(sphere));
			return sphere;
		}

		virtual XNA::Frustum GetBoundsFrustum() const
		{
			XNA::Frustum frustum;
			memset(&frustum, 0, sizeof(frustum));
			return frustum;
		}

		void SetBoundsAABBCache(const XNA::AxisAlignedBox & aabb)
		{
			_aabbCache = aabb;
		}

		const XNA::AxisAlignedBox & GetBoundsAABBCache() const
		{
			return _aabbCache;
		}

		void SetBoundsOBBCache(const XNA::OrientedBox & obb)
		{
			_obbCache = obb;
		}

		const XNA::OrientedBox & GetBoundsOBBCache() const
		{
			return _obbCache;
		}

		void SetBoundsSphereCache(const XNA::Sphere & sphere)
		{
			_sphereCache = sphere;
		}

		const XNA::Sphere & GetBoundsSphereCache() const
		{
			return _sphereCache;
		}

		void SetBoundsFrustumCache(const XNA::Frustum & frustum)
		{
			_frustumCache = frustum;
		}

		const XNA::Frustum & GetBoundsFrustumCache() const
		{
			return _frustumCache;
		}

		void SetCullState(bool cullState)
		{
			_cullState = cullState;
		}

		bool GetCullState() const
		{
			return _cullState;
		}

	protected:
		XNA::AxisAlignedBox _aabbCache;
		XNA::OrientedBox _obbCache;
		XNA::Sphere _sphereCache;
		XNA::Frustum _frustumCache;
		bool _cullState;
	};

	class TOYGE_CORE_API SceneCuller
	{
	public:
		virtual XNA::AxisAlignedBox GetSceneAABB() = 0;

		virtual void AddElement(const Ptr<Cullable> & element) = 0;

		virtual void RemoveElement(const Ptr<Cullable> & emelent) = 0;

		virtual void UpdateElement(const Ptr<Cullable> & element) = 0;

		virtual void GetAllElements(std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const XNA::AxisAlignedBox & aabb, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const XNA::OrientedBox & obb, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const XNA::Frustum & frustum, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const std::vector<XMFLOAT4> & frustumPlanes, std::vector<Ptr<Cullable>> & outElements) = 0;

		virtual void Cull(const XNA::Sphere & sphere, std::vector<Ptr<Cullable>> & outElements) = 0;
	};
}

#endif