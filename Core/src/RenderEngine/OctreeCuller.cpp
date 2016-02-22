#include "ToyGE\RenderEngine\OctreeCuller.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	OctreeCuller::OctreeCuller(int32_t maxNodeDepth, int32_t maxNodeElements)
		: _maxNodeDepth(maxNodeDepth),
		_maxNodeElements(maxNodeElements),
		_depth(0),
		_numElementsWithinNode(0),
		_bNeedRebuild(false)
	{
		_nodeAABB.min = 0.0f;
		_nodeAABB.max = 0.0f;

		_nodeID = 0;

		_allElements = nullptr;
		_allNodes = nullptr;

		_bRoot = true;
	}

	OctreeCuller::~OctreeCuller()
	{
		if (IsRoot())
		{
			delete _allElements;
			delete _allNodes;

			_allElements = nullptr;
			_allNodes = nullptr;
		}
	}

	void OctreeCuller::SetParent(const Ptr<SceneCuller> & parent)
	{
		_parent = parent;
	}

	void OctreeCuller::ReBuild()
	{
		if (!IsRoot())
			return;
		if (!_allElements || _allElements->size() == 0)
			return;

		_bNeedRebuild = false;

		_nodeAABB = ComputeNodeAABB(*_allElements);

		auto tmp = *_allElements;

		// Remove all nodes
		for (auto & sub : _subCullers)
			sub.reset();
		_elements.clear();
		_numElementsWithinNode = 0;
		_allElements->clear();

		// Add all elements
		for (auto & elem : tmp)
			AddElement(elem);
	}

	void OctreeCuller::AddElement(const Ptr<Cullable> & element)
	{
		// Check Is Needed Rebuilt
		if (IsRoot())
		{
			if (!_allElements)
				_allElements = new std::vector<Ptr<Cullable>>();
			if (!_allNodes)
			{
				_allNodes = new std::vector<Ptr<SceneCuller>>();
				_allNodes->push_back(nullptr); // push null for root
			}

			if (std::find(_allElements->begin(), _allElements->end(), element) != _allElements->end())
				return;

			_allElements->push_back(element);
			++_numElementsWithinNode;

			if (IsExceedNodeAABB(element))
				_bNeedRebuild = true;

			if (_bNeedRebuild)
				return;
		}

		// Check intersection
		if (!Intersect(element))
			return;

		if (!IsRoot())
			++_numElementsWithinNode;

		if (!HasSubNodes())
		{
			if (static_cast<int32_t>(_elements.size()) < _maxNodeElements)
			{
				_elements.push_back(element);
				element->cullerHandle = (int32_t)_nodeID;
			}
			// This node is full
			else
			{
				// Split when not too deep
				if (_depth < _maxNodeDepth)
				{
					Split();
					// Add this element and this node's elements to sub nodes
					for (auto & sub : _subCullers)
					{
						sub->AddElement(element);
						for (auto & elem : _elements)
							sub->AddElement(elem);
					}
					_elements.clear();
				}
				else
				{
					_elements.push_back(element);
				}
			}
		}
		else
		{
			for (auto & sub : _subCullers)
				sub->AddElement(element);
		}
	}

	void OctreeCuller::RemoveElement(const Ptr<Cullable> & element)
	{
		if (!element || element->cullerHandle < 0 || !_allElements)
			return;

		if (IsRoot())
		{
			auto removeElemItr = std::find(_allElements->begin(), _allElements->end(), element);
			if (removeElemItr != _allElements->end())
			{
				_allElements->erase(removeElemItr);
				--_numElementsWithinNode;
			}

			if (element->cullerHandle == 0)
			{
				auto removeElemItr = std::find(_elements.begin(), _elements.end(), element);
				if (removeElemItr != _elements.end())
					_elements.erase(removeElemItr);
				element->cullerHandle = -1;
			}
			else
			{
				auto node = (*_allNodes)[element->cullerHandle];
				node->RemoveElement(element);
			}
		}
		else
		{
			auto removeElemItr = std::find(_elements.begin(), _elements.end(), element);
			if (removeElemItr != _elements.end())
				_elements.erase(removeElemItr);
			element->cullerHandle = -1;
		}

		////Check intersection
		//if (!IntersectCache(element))
		//	return;

		//if (!IsRoot())
		//	--_numElementsWithinNode;

		//if (HasSubNodes())
		//{
		//	if (_numElementsWithinNode <= _maxNodeElements)
		//	{
		//		GetAllElements(_elements);
		//	}
		//	else
		//	{
		//		for (auto & sub : _subCullers)
		//			sub->RemoveElement(element);
		//	}
		//}
		//else
		//{
		//	auto removeElemItr = std::find(_elements.begin(), _elements.end(), element);
		//	if (removeElemItr != _elements.end())
		//		_elements.erase(removeElemItr);
		//}
	}

	void OctreeCuller::UpdateElement(const Ptr<Cullable> & element)
	{
		RemoveElement(element);
		AddElement(element);
	}

	void OctreeCuller::GetAllElements(std::vector<Ptr<Cullable>> & outElements)
	{
		if (IsRoot())
		{
			if(_allElements)
				outElements = *_allElements;
		}
		else
		{
			outElements.clear();
			if (HasSubNodes())
			{
				for (auto & sub : _subCullers)
					sub->GetAllElements(outElements);
			}
			else
			{
				for (auto & elem : _elements)
				{
					if (std::find(outElements.begin(), outElements.end(), elem) == outElements.end())
						outElements.push_back(elem);
				}
			}
		}

	}

	void OctreeCuller::Cull(const AABBox & aabb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<AABBox>(
			aabb,
			[](const AABBox * pVolumeA, const AABBox * pVolumeB) -> bool
		{
			return intersect_aabb_aabb(*pVolumeA, *pVolumeB);
		},
			outElements);
	}

	void OctreeCuller::Cull(const OBBox & obb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<OBBox>(
			obb,
			[](const AABBox * pVolumeA, const OBBox * pVolumeB) -> bool
		{
			return intersect_aabb_obb(*pVolumeA, *pVolumeB);
		}, 
			outElements);
	}

	void OctreeCuller::Cull(const Frustum & frustum, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<Frustum>(
			frustum,
			[](const AABBox * pVolumeA, const Frustum * pVolumeB) -> bool
		{
			return intersect_aabb_frustum(*pVolumeA, *pVolumeB) != BO_NO;
		},
			outElements);
	}

	void OctreeCuller::Cull(const Sphere & sphere, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<Sphere>(
			sphere,
			[](const AABBox * pVolumeA, const Sphere * pVolumeB) -> bool
		{
			return intersect_aabb_sphere(*pVolumeA, *pVolumeB);
		},
		outElements);
	}

	void OctreeCuller::Split()
	{
		float3 extents = _nodeAABB.Extents() * 0.5f;
		float3 center;

		for (int32_t subIndex = 0; subIndex < 8; ++subIndex)
		{
			auto subCuller = CreateNode(_maxNodeDepth, _maxNodeElements);

			subCuller->_parent = shared_from_this();
			subCuller->_depth = _depth + 1;
			subCuller->_allElements = _allElements;
			subCuller->_allNodes = _allNodes;
			subCuller->_bRoot = false;

			center.x() = _nodeAABB.Center().x() + (       (subIndex & 1UL) ? extents.x() : -extents.x());
			center.y() = _nodeAABB.Center().y() + ((subIndex & (1UL << 1)) ? extents.y() : -extents.y());
			center.z() = _nodeAABB.Center().z() + ((subIndex & (1UL << 2)) ? extents.z() : -extents.z());
			subCuller->_nodeAABB = AABBox(center - extents, center + extents);

			_subCullers[subIndex] = subCuller;

			subCuller->_nodeID = (uint32_t)_allNodes->size();
			_allNodes->push_back(subCuller);
		}
	}

	template <typename BoundsType>
	void OctreeCuller::_Cull(
		const BoundsType & bounds,
		const std::function<bool(const AABBox *, const BoundsType *)> & intersectFunc,
		std::vector<Ptr<Cullable>> & outElements)
	{
		if (_bNeedRebuild)
			ReBuild();

		if (!intersectFunc(&_nodeAABB, &bounds))
			return;

		if (HasSubNodes())
		{
			for (auto & sub : _subCullers)
				sub->Cull(bounds, outElements);
		}
		else
		{
			for (auto & elem : _elements)
			{
				if (elem->cullState == false)
				{
					elem->cullState = true;
					outElements.push_back(elem);
				}
			}
		}

		if (IsRoot())
		{
			for (auto & elem : outElements)
				elem->cullState = false;
		}
	}


	/*
	DefaultRenderObjectCuller
	*/
	std::shared_ptr<OctreeCuller> DefaultRenderObjectCuller::CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements)
	{
		return std::make_shared<DefaultRenderObjectCuller>(maxNodeDepth, maxNodeElements);
	}

	AABBox DefaultRenderObjectCuller::ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements)
	{
		if (elements.size() == 0)
			return AABBox();

		float3 nodeAABBMin = (*elements.begin())->GetBoundsAABB().min;
		float3 nodeAABBMax = (*elements.begin())->GetBoundsAABB().max;
		for (auto itr = ++elements.begin(); itr != elements.end(); ++itr)
		{
			nodeAABBMin = min_vec(nodeAABBMin, (*itr)->GetBoundsAABB().min);
			nodeAABBMax = max_vec(nodeAABBMax, (*itr)->GetBoundsAABB().max);
		}

		nodeAABBMin -= 1e-4f;
		nodeAABBMax += 1e-4f;
		return AABBox(nodeAABBMin, nodeAABBMax);
	}

	bool DefaultRenderObjectCuller::IsExceedNodeAABB(const Ptr<Cullable> & element)
	{
		return !_nodeAABB.Contains(element->GetBoundsAABB());
	}

	bool DefaultRenderObjectCuller::Intersect(const Ptr<Cullable> & element)
	{
		return intersect_aabb_aabb(_nodeAABB, element->GetBoundsAABB());
	}


	/*
	DefaultRenderLightCuller
	*/
	void DefaultRenderLightCuller::AddElement(const Ptr<Cullable> & element)
	{
		if (IsRoot())
		{
			if (!_allElements)
				_allElements = new std::vector<Ptr<Cullable>>();
			if (!_allNodes)
			{
				_allNodes = new std::vector<Ptr<SceneCuller>>();
				_allNodes->push_back(nullptr); // push null for root
			}

			auto light = std::static_pointer_cast<LightComponent>(element);
			if (light->Type() == LIGHT_DIRECTIONAL)
			{
				auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
				if (std::find(_rootDirLights.begin(), _rootDirLights.end(), dirLight) == _rootDirLights.end())
				{
					_rootDirLights.push_back(dirLight);
				}
				return;
			}
		}
		DefaultRenderObjectCuller::AddElement(element);
	}

	void DefaultRenderLightCuller::RemoveElement(const Ptr<Cullable> & element)
	{
		if (IsRoot())
		{
			auto light = std::static_pointer_cast<LightComponent>(element);
			if (light->Type() == LIGHT_DIRECTIONAL)
			{
				auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
				auto dirLightFind = std::find(_rootDirLights.begin(), _rootDirLights.end(), dirLight);
				if (dirLightFind != _rootDirLights.end())
				{
					_rootDirLights.erase(dirLightFind);
				}
				return;
			}
		}
		DefaultRenderObjectCuller::RemoveElement(element);
	}

	void DefaultRenderLightCuller::UpdateElement(const Ptr<Cullable> & element)
	{
		if (IsRoot())
		{
			auto light = std::static_pointer_cast<LightComponent>(element);
			if (light->Type() == LIGHT_DIRECTIONAL)
			{
				return;
			}
		}
		DefaultRenderObjectCuller::UpdateElement(element);
	}

	void DefaultRenderLightCuller::GetAllElements(std::vector<Ptr<Cullable>> & outElements)
	{
		DefaultRenderObjectCuller::GetAllElements(outElements);
		if (IsRoot())
		{
			for (auto & light : _rootDirLights)
				outElements.push_back(light);
		}
	}

	void DefaultRenderLightCuller::Cull(const AABBox & aabb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<AABBox>(
			aabb, 
			[](const AABBox * lightAABB, const AABBox * aabb) -> bool
		{
			return intersect_aabb_aabb(*lightAABB, *aabb);
		}, 
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const OBBox & obb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<OBBox>(
			obb,
			[](const AABBox * lightAABB, const OBBox * obb) -> bool
		{
			return intersect_aabb_obb(*lightAABB, *obb);
		},
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const Frustum & frustum, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<Frustum>(
			frustum,
			[](const AABBox * pVolumeA, const Frustum * pVolumeB) -> bool
		{
			return intersect_aabb_frustum(*pVolumeA, *pVolumeB) != BO_NO;
		},
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const Sphere & sphere, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<Sphere>(
			sphere,
			[](const AABBox * lightAABB, const Sphere * sp) -> bool
		{
			return intersect_aabb_sphere(*lightAABB, *sp);
		},
			outElements);
	}

	std::shared_ptr<OctreeCuller> DefaultRenderLightCuller::CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements)
	{
		return std::make_shared<DefaultRenderLightCuller>(maxNodeDepth, maxNodeElements);
	}

	AABBox DefaultRenderLightCuller::ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements)
	{
		if (elements.size() == 0)
			return AABBox();

		float3 nodeAABBMin = (*elements.begin())->GetBoundsAABB().min;
		float3 nodeAABBMax = (*elements.begin())->GetBoundsAABB().max;
		for (auto itr = ++elements.begin(); itr != elements.end(); ++itr)
		{
			auto light = std::static_pointer_cast<LightComponent>(*itr);
			if (light->Type() == LIGHT_DIRECTIONAL)
			{
				auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
				if (dirLight->IsInfluenceAll())
					continue;
			}

			nodeAABBMin = min_vec(nodeAABBMin, (*itr)->GetBoundsAABB().min);
			nodeAABBMax = max_vec(nodeAABBMax, (*itr)->GetBoundsAABB().max);
		}

		nodeAABBMin -= 1e-4f;
		nodeAABBMax += 1e-4f;
		return AABBox(nodeAABBMin, nodeAABBMax);
	}

	bool DefaultRenderLightCuller::IsExceedNodeAABB(const Ptr<Cullable> & element)
	{
		auto light = std::static_pointer_cast<LightComponent>(element);
		if (light->Type() == LIGHT_DIRECTIONAL)
		{
			auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
			if (dirLight->IsInfluenceAll())
				return false;
		}
		return !_nodeAABB.Contains(element->GetBoundsAABB());
	}

	template <typename BoundsType>
	void DefaultRenderLightCuller::_Cull(
		const BoundsType & bounds,
		const std::function<bool(const AABBox *, const BoundsType *)> & intersectFunc,
		std::vector<Ptr<Cullable>> & outElements)
	{
		if (IsRoot())
		{
			for (auto & light : _rootDirLights)
			{
				if (light->IsInfluenceAll())
				{
					outElements.push_back(light);
				}
				else
				{
					if (intersectFunc(&light->GetBoundsAABB(), &bounds))
						outElements.push_back(light);
				}
			}
		}
		DefaultRenderObjectCuller::Cull(bounds, outElements);
	}
}