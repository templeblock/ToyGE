#pragma once
#ifndef OCTREECULLER_H
#define OCTREECULLER_H

#include "ToyGE\RenderEngine\SceneCuller.h"

namespace ToyGE
{
	class DirectionalLightComponent;

	class TOYGE_CORE_API OctreeCuller : public SceneCuller, public std::enable_shared_from_this<OctreeCuller>
	{
	public:
		OctreeCuller(int32_t maxNodeDepth, int32_t maxNodeElements);

		virtual ~OctreeCuller();

		void SetParent(const Ptr<SceneCuller> & parent);

		bool HasSubNodes() const
		{
			return _subCullers[0] != nullptr;
		}

		bool IsRoot() const
		{
			return _bRoot;
		}

		int32_t Depth() const
		{
			return _depth;
		}

		AABBox GetSceneAABB() override
		{
			if (IsRoot())
				return _nodeAABB;
			else
				return _parent.lock()->GetSceneAABB();
		}

		void ReBuild();

		void AddElement(const Ptr<Cullable> & element) override;

		void RemoveElement(const Ptr<Cullable> & element) override;

		void UpdateElement(const Ptr<Cullable> & element) override;

		void GetAllElements(std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const AABBox & aabb, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const OBBox & obb, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const Frustum & frustum, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const Sphere & sphere, std::vector<Ptr<Cullable>> & outElements) override;

	protected:
		const int32_t _maxNodeDepth;
		const int32_t _maxNodeElements;

		bool _bRoot;
		uint32_t _nodeID;
		int32_t _depth;
		int32_t _numElementsWithinNode;
		std::weak_ptr<SceneCuller> _parent;
		std::array<Ptr<SceneCuller>, 8> _subCullers;
		std::vector<Ptr<Cullable>> _elements;
		AABBox _nodeAABB;


		std::vector<Ptr<SceneCuller>> * _allNodes;
		std::vector<Ptr<Cullable>> * _allElements;
		bool _bNeedRebuild;

		virtual void Split();

		virtual std::shared_ptr<OctreeCuller> CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements) = 0;

		virtual AABBox ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements) = 0;
		
		virtual bool IsExceedNodeAABB(const Ptr<Cullable> & element) = 0;

		virtual bool Intersect(const Ptr<Cullable> & element) = 0;

	private:
		template <typename BoundsType>
		void _Cull(
			const BoundsType & bounds,
			const std::function<bool(const AABBox *, const BoundsType *)> & intersectFunc,
			std::vector<Ptr<Cullable>> & outElements);
	};

	class TOYGE_CORE_API DefaultRenderObjectCuller : public OctreeCuller
	{
	public:
		//using OctreeCuller::OctreeCuller;
		DefaultRenderObjectCuller(int32_t maxNodeDepth, int32_t maxNodeElements)
			: OctreeCuller(maxNodeDepth, maxNodeElements)
		{

		}

		DefaultRenderObjectCuller()
			: DefaultRenderObjectCuller(4, 1)
		{

		}

	protected:
		std::shared_ptr<OctreeCuller> CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements) override;

		AABBox ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements) override;

		bool IsExceedNodeAABB(const Ptr<Cullable> & element) override;

		bool Intersect(const Ptr<Cullable> & element) override;
	};

	class TOYGE_CORE_API DefaultRenderLightCuller : public DefaultRenderObjectCuller
	{
	public:
		DefaultRenderLightCuller(int32_t maxNodeDepth, int32_t maxNodeElements)
			: DefaultRenderObjectCuller(maxNodeDepth, maxNodeElements)
		{

		}

		DefaultRenderLightCuller()
			: DefaultRenderLightCuller(4, 1)
		{

		}

		void AddElement(const Ptr<Cullable> & element) override;

		void RemoveElement(const Ptr<Cullable> & element) override;

		void UpdateElement(const Ptr<Cullable> & element) override;

		void GetAllElements(std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const AABBox & aabb, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const OBBox & obb, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const Frustum & frustum, std::vector<Ptr<Cullable>> & outElements) override;

		void Cull(const Sphere & sphere, std::vector<Ptr<Cullable>> & outElements) override;

	protected:
		std::shared_ptr<OctreeCuller> CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements) override;

		AABBox ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements) override;

		bool IsExceedNodeAABB(const Ptr<Cullable> & element) override;

	private:
		std::vector<Ptr<DirectionalLightComponent>> _rootDirLights;

		template <typename BoundsType>
		void _Cull(
			const BoundsType & bounds,
			const std::function<bool(const AABBox *, const BoundsType *)> & intersectFunc,
			std::vector<Ptr<Cullable>> & outElements);
	};
}

#endif