#include "ToyGE\RenderEngine\OctreeCuller.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\LightComponent.h"

namespace ToyGE
{
	static bool Contains(const XNA::AxisAlignedBox & aabb0, const XNA::AxisAlignedBox & aabb1)
	{
		float3 min_0;
		float3 max_0;
		Math::AxisAlignedBoxToMinMax(aabb0, min_0, max_0);

		float3 min_1;
		float3 max_1;
		Math::AxisAlignedBoxToMinMax(aabb1, min_1, max_1);

		return min_0 <= min_1 && max_0 >= max_1;
	}

	OctreeCuller::OctreeCuller(int32_t maxNodeDepth, int32_t maxNodeElements)
		: _maxNodeDepth(maxNodeDepth),
		_maxNodeElements(maxNodeElements),
		_depth(0),
		_numElementsWithinNode(0),
		_bNeedRebuild(false)
	{
		memset(&_nodeAABB, 0, sizeof(_nodeAABB));
	}

	void OctreeCuller::SetParent(const Ptr<SceneCuller> & parent)
	{
		_parent = parent;
	}

	void OctreeCuller::ReBuild()
	{
		if (!IsRoot())
			return;
		if (_rootAllElements.size() == 0)
			return;

		_nodeAABB = ComputeNodeAABB(_rootAllElements);

		auto tmp = _rootAllElements;
		//Clear
		for (auto & sub : _subCullers)
			sub.reset();
		_elements.clear();
		_numElementsWithinNode = 0;
		_rootAllElements.clear();

		_bNeedRebuild = false;

		for (auto & elem : tmp)
			AddElement(elem);
	}

	void OctreeCuller::AddElement(const Ptr<Cullable> & element)
	{
		//Check Is Needed Rebuilt
		if (IsRoot())
		{
			//_rootAllElements.insert(element);
			if (std::find(_rootAllElements.begin(), _rootAllElements.end(), element) != _rootAllElements.end())
				return;

			SetElementBoundsCache(element);

			_rootAllElements.push_back(element);
			++_numElementsWithinNode;
			if (_bNeedRebuild)
				return;

			if (IsExceedNodeAABB(element))
			{
				_bNeedRebuild = true;
				return;
			}
		}

		//Check intersection
		if (!Intersect(element))
			return;

		if (!IsRoot())
			++_numElementsWithinNode;

		if (!HasSubNodes())
		{
			if (static_cast<int32_t>(_elements.size()) < _maxNodeElements)
			{
				//_elements.insert(element);
				_elements.push_back(element);
			}
			//Objs Full
			else
			{
				//Split when not too deep
				if (_depth < _maxNodeDepth)
				{
					Split();
					//Add this element and this node's elements to sub nodes
					for (auto & sub : _subCullers)
					{
						sub->AddElement(element);
						for (auto & ele : _elements)
							sub->AddElement(ele);
					}
					_elements.clear();
				}
				else
				{
					//_elements.insert(element);
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
		if (IsRoot())
		{
			//_rootAllElements.erase(_rootAllElements.find(element));
			auto removeElemItr = std::find(_rootAllElements.begin(), _rootAllElements.end(), element);
			if (removeElemItr != _rootAllElements.end())
			{
				_rootAllElements.erase(removeElemItr);
				--_numElementsWithinNode;
			}
		}

		//Check intersection
		if (!IntersectCache(element))
			return;

		if (!IsRoot())
			--_numElementsWithinNode;

		if (HasSubNodes())
		{
			if (_numElementsWithinNode <= _maxNodeElements)
			{
				GetAllElements(_elements);
			}
			else
			{
				for (auto & sub : _subCullers)
					sub->RemoveElement(element);
			}
		}
		else
		{
			auto removeElemItr = std::find(_elements.begin(), _elements.end(), element);
			if (removeElemItr != _elements.end())
				_elements.erase(removeElemItr);
		}
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
			outElements = _rootAllElements;
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

	void OctreeCuller::Cull(const XNA::AxisAlignedBox & aabb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::AxisAlignedBox>(
			aabb,
			[](const XNA::AxisAlignedBox * pVolumeA, const XNA::AxisAlignedBox * pVolumeB) -> bool
		{
			return XNA::IntersectAxisAlignedBoxAxisAlignedBox(pVolumeA, pVolumeB) == TRUE;
		},
			outElements);
	}

	void OctreeCuller::Cull(const XNA::OrientedBox & obb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::OrientedBox>(
			obb,
			[](const XNA::AxisAlignedBox * pVolumeA, const XNA::OrientedBox * pVolumeB) -> bool
		{
			return XNA::IntersectAxisAlignedBoxOrientedBox(pVolumeA, pVolumeB) == TRUE;
		}, 
			outElements);
	}

	void OctreeCuller::Cull(const XNA::Frustum & frustum, std::vector<Ptr<Cullable>> & outElements)
	{
		XMVECTOR p0XM;
		XMVECTOR p1XM;
		XMVECTOR p2XM;
		XMVECTOR p3XM;
		XMVECTOR p4XM;
		XMVECTOR p5XM;
		ComputePlanesFromFrustum(&frustum, &p0XM, &p1XM, &p2XM, &p3XM, &p4XM, &p5XM);

		XMFLOAT4 p0;
		XMFLOAT4 p1;
		XMFLOAT4 p2;
		XMFLOAT4 p3;
		XMFLOAT4 p4;
		XMFLOAT4 p5;
		XMStoreFloat4(&p0, p0XM);
		XMStoreFloat4(&p1, p1XM);
		XMStoreFloat4(&p2, p2XM);
		XMStoreFloat4(&p3, p3XM);
		XMStoreFloat4(&p4, p4XM);
		XMStoreFloat4(&p5, p5XM);
		Cull({ p0, p1, p2, p3, p4, p5 }, outElements);
	}

	void OctreeCuller::Cull(const std::vector<XMFLOAT4> & frustumPlanes, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<std::vector<XMFLOAT4>>(
			frustumPlanes,
			[](const XNA::AxisAlignedBox * pVolumeA, const std::vector<XMFLOAT4> * pVolumeB) -> bool
		{
			return XNA::IntersectAxisAlignedBoxFrustum2(pVolumeA, pVolumeB) > 0;
		},
			outElements);
	}

	void OctreeCuller::Cull(const XNA::Sphere & sphere, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::Sphere>(
			sphere,
			[](const XNA::AxisAlignedBox * pVolumeA, const XNA::Sphere * pVolumeB) -> bool
		{
			return XNA::IntersectSphereAxisAlignedBox(pVolumeB, pVolumeA) == TRUE;
		},
		outElements);
	}

	void OctreeCuller::Split()
	{
		XNA::AxisAlignedBox subAABB;
		subAABB.Extents.x = _nodeAABB.Extents.x * 0.5f;
		subAABB.Extents.y = _nodeAABB.Extents.y * 0.5f;
		subAABB.Extents.z = _nodeAABB.Extents.z * 0.5f;
		for (int32_t subIndex = 0; subIndex < 8; ++subIndex)
		{
			//auto subCuller = std::make_shared<OctreeCuller>(_maxNodeDepth, _maxNodeElements);
			auto subCuller = CreateNode(_maxNodeDepth, _maxNodeElements);
			subCuller->SetParent(shared_from_this());
			subCuller->_depth = _depth + 1;
			subAABB.Center.x = _nodeAABB.Center.x + (       (subIndex & 1UL) ? subAABB.Extents.x : -subAABB.Extents.x);
			subAABB.Center.y = _nodeAABB.Center.y + ((subIndex & (1UL << 1)) ? subAABB.Extents.y : -subAABB.Extents.y);
			subAABB.Center.z = _nodeAABB.Center.z + ((subIndex & (1UL << 2)) ? subAABB.Extents.z : -subAABB.Extents.z);
			subCuller->_nodeAABB = subAABB;
			_subCullers[subIndex] = subCuller;
		}
	}

	template <typename BoundsType>
	void OctreeCuller::_Cull(
		const BoundsType & bounds,
		const std::function<bool(const XNA::AxisAlignedBox *, const BoundsType *)> & intersectFunc,
		std::vector<Ptr<Cullable>> & outElements)
	{
		if (_bNeedRebuild)
			ReBuild();

		if (IsRoot())
		{
			for (auto & elem : _rootAllElements)
				elem->SetCullState(false);
		}

		if (!intersectFunc(&_nodeAABB, &bounds))
			return;

		if (HasSubNodes())
		{
			for (auto & sub : _subCullers)
				sub->Cull(bounds, outElements);
				//sub->_Cull<BoundsType>(bounds, intersectFunc, outElements);
		}
		else
		{
			//outElements.insert(_elements.begin(), _elements.end());
			for (auto & elem : _elements)
			{
				if (elem->GetCullState() == false)
				{
					elem->SetCullState(true);
					outElements.push_back(elem);
				}
			}
		}
	}


	/*
	DefaultRenderObjectCuller
	*/
	std::shared_ptr<OctreeCuller> DefaultRenderObjectCuller::CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements)
	{
		return std::make_shared<DefaultRenderObjectCuller>(maxNodeDepth, maxNodeElements);
	}

	XNA::AxisAlignedBox DefaultRenderObjectCuller::ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements)
	{
		float3 nodeAABBMin;
		float3 nodeAABBMax;
		Math::AxisAlignedBoxToMinMax((*elements.begin())->GetBoundsAABB(), nodeAABBMin, nodeAABBMax);
		for (auto itr = ++elements.begin(); itr != elements.end(); ++itr)
		{
			float3 elementAABBMin;
			float3 elementAABBMax;
			Math::AxisAlignedBoxToMinMax((*itr)->GetBoundsAABB(), elementAABBMin, elementAABBMax);
			nodeAABBMin = vecMin(nodeAABBMin, elementAABBMin);
			nodeAABBMax = vecMax(nodeAABBMax, elementAABBMax);
		}

		nodeAABBMin -= 1e-4f;
		nodeAABBMax += 1e-4f;
		XNA::AxisAlignedBox nodeAABB;
		Math::MinMaxToAxisAlignedBox(nodeAABBMin, nodeAABBMax, nodeAABB);
		return nodeAABB;
	}

	void DefaultRenderObjectCuller::SetElementBoundsCache(const Ptr<Cullable> & element)
	{
		element->SetBoundsAABBCache(element->GetBoundsAABB());
	}

	bool DefaultRenderObjectCuller::IsExceedNodeAABB(const Ptr<Cullable> & element)
	{
		return !Contains(_nodeAABB, element->GetBoundsAABB());
	}

	bool DefaultRenderObjectCuller::Intersect(const Ptr<Cullable> & element)
	{
		return XNA::IntersectAxisAlignedBoxAxisAlignedBox(&_nodeAABB, &element->GetBoundsAABB()) == TRUE;
	}

	bool DefaultRenderObjectCuller::IntersectCache(const Ptr<Cullable> & element)
	{
		return XNA::IntersectAxisAlignedBoxAxisAlignedBox(&_nodeAABB, &element->GetBoundsAABBCache()) == TRUE;
	}


	/*
	DefaultRenderLightCuller
	*/
	void DefaultRenderLightCuller::AddElement(const Ptr<Cullable> & element)
	{
		if (IsRoot())
		{
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

	void DefaultRenderLightCuller::Cull(const XNA::AxisAlignedBox & aabb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::AxisAlignedBox>(
			aabb, 
			[](const XNA::AxisAlignedBox * lightAABB, const XNA::AxisAlignedBox * aabb) -> bool
		{
			return XNA::IntersectAxisAlignedBoxAxisAlignedBox(lightAABB, aabb) == TRUE;
		}, 
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const XNA::OrientedBox & obb, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::OrientedBox>(
			obb,
			[](const XNA::AxisAlignedBox * lightAABB, const XNA::OrientedBox * obb) -> bool
		{
			return XNA::IntersectAxisAlignedBoxOrientedBox(lightAABB, obb) == TRUE;
		},
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const XNA::Frustum & frustum, std::vector<Ptr<Cullable>> & outElements)
	{
		XMVECTOR p0XM;
		XMVECTOR p1XM;
		XMVECTOR p2XM;
		XMVECTOR p3XM;
		XMVECTOR p4XM;
		XMVECTOR p5XM;
		ComputePlanesFromFrustum(&frustum, &p0XM, &p1XM, &p2XM, &p3XM, &p4XM, &p5XM);

		XMFLOAT4 p0;
		XMFLOAT4 p1;
		XMFLOAT4 p2;
		XMFLOAT4 p3;
		XMFLOAT4 p4;
		XMFLOAT4 p5;
		XMStoreFloat4(&p0, p0XM);
		XMStoreFloat4(&p1, p1XM);
		XMStoreFloat4(&p2, p2XM);
		XMStoreFloat4(&p3, p3XM);
		XMStoreFloat4(&p4, p4XM);
		XMStoreFloat4(&p5, p5XM);
		Cull({ p0, p1, p2, p3, p4, p5 }, outElements);
	}

	void DefaultRenderLightCuller::Cull(const std::vector<XMFLOAT4> & frustumPlanes, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<std::vector<XMFLOAT4>>(
			frustumPlanes,
			[](const XNA::AxisAlignedBox * lightAABB, const std::vector<XMFLOAT4> * planes) -> bool
		{
			return XNA::IntersectAxisAlignedBoxFrustum2(lightAABB, planes) > 0;
		},
			outElements);
	}

	void DefaultRenderLightCuller::Cull(const XNA::Sphere & sphere, std::vector<Ptr<Cullable>> & outElements)
	{
		_Cull<XNA::Sphere>(
			sphere,
			[](const XNA::AxisAlignedBox * lightAABB, const XNA::Sphere * sp) -> bool
		{
			return XNA::IntersectSphereAxisAlignedBox(sp, lightAABB) == TRUE;
		},
			outElements);
	}

	std::shared_ptr<OctreeCuller> DefaultRenderLightCuller::CreateNode(int32_t maxNodeDepth, int32_t maxNodeElements)
	{
		return std::make_shared<DefaultRenderLightCuller>(maxNodeDepth, maxNodeElements);
	}

	//XNA::AxisAlignedBox DefaultRenderLightCuller::ComputeNodeAABB(const std::vector<Ptr<Cullable>> & elements)
	//{
	//	float3 nodeAABBMin = FLT_MAX;
	//	float3 nodeAABBMax = FLT_MIN;
	//	//AxisAlignedBoxToMinMax((*elements.begin())->GetBoundsAABB(), nodeAABBMin, nodeAABBMax);
	//	bool bNodeAABBValid = false;
	//	for (auto itr = elements.begin(); itr != elements.end(); ++itr)
	//	{
	//		auto light = std::static_pointer_cast<LightComponent>(*itr);
	//		if (light->Type() == LIGHT_DIRECTIONAL)
	//		{
	//			auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
	//			if (dirLight->IsInfluenceAll())
	//				continue;
	//		}

	//		float3 elementAABBMin;
	//		float3 elementAABBMax;
	//		Math::AxisAlignedBoxToMinMax((*itr)->GetBoundsAABB(), elementAABBMin, elementAABBMax);
	//		nodeAABBMin = vecMin(nodeAABBMin, elementAABBMin);
	//		nodeAABBMax = vecMax(nodeAABBMax, elementAABBMax);
	//		bNodeAABBValid = true;
	//	}
	//	if (!bNodeAABBValid)
	//	{
	//		nodeAABBMin = nodeAABBMax = 0.0f;
	//	}

	//	XNA::AxisAlignedBox nodeAABB;
	//	Math::MinMaxToAxisAlignedBox(nodeAABBMin, nodeAABBMax, nodeAABB);
	//	return nodeAABB;
	//}

	//bool DefaultRenderLightCuller::IsExceedNodeAABB(const Ptr<Cullable> & element)
	//{
	//	auto light = std::static_pointer_cast<LightComponent>(element);
	//	if (light->Type() == LIGHT_DIRECTIONAL)
	//	{
	//		auto dirLight = std::static_pointer_cast<DirectionalLightComponent>(light);
	//		if (dirLight->IsInfluenceAll())
	//			return false;
	//	}
	//	return !Contains(_nodeAABB, element->GetBoundsAABB());
	//}

	template <typename BoundsType>
	void DefaultRenderLightCuller::_Cull(
		const BoundsType & bounds,
		const std::function<bool(const XNA::AxisAlignedBox *, const BoundsType *)> & intersectFunc,
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