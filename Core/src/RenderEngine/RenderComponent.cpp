#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
	RenderComponent::RenderComponent()
		: _bCastShadows(true),
		_bCastCaustics(true)
	{
		memset(&_localAABB, 0, sizeof(_localAABB));
		memset(&_boundsAABB, 0, sizeof(_boundsAABB));
	}

	String RenderComponent::Name()
	{
		return "RENDERCOMPONENT";
	}

	String RenderComponent::GetComponentName() const
	{
		return RenderComponent::Name();
	}

	XNA::AxisAlignedBox RenderComponent::GetBoundsAABB() const
	{
		return _boundsAABB;
	}

	void RenderComponent::DoActive()
	{
		auto renderObjCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		renderObjCuller->AddElement(shared_from_this());
	}

	void RenderComponent::OnTranformUpdated()
	{
		UpdateBoundsAABB();
		if (IsActive())
		{
			Global::GetRenderEngine()->GetSceneRenderObjsCuller()->UpdateElement(shared_from_this());
		}
	}

	void RenderComponent::UpdateLocalAABB()
	{
		std::vector<XMFLOAT3> points;
		for (int32_t slotIndex = 0; slotIndex != _mesh->NumVertexSlots(); ++slotIndex)
		{
			auto posDescIndex = _mesh->GetVertexData(slotIndex).FindElementDesc(StandardVertexElementName::Position());
			if (posDescIndex >= 0)
			{
				auto & vertexData = _mesh->GetVertexData(slotIndex);
				auto & posDesc = vertexData.elementsDesc[posDescIndex];
				for (int32_t i = 0; i != vertexData.numVertices; ++i)
				{
					auto & v = vertexData.GetElement<float3>(i, posDesc);
					points.push_back(XMFLOAT3(v.x, v.y, v.z));
				}
			}
		}

		XNA::ComputeBoundingAxisAlignedBoxFromPoints(&_localAABB, static_cast<UINT>(points.size()), &points[0], sizeof(XMFLOAT3));
	}

	void RenderComponent::UpdateBoundsAABB()
	{
		float3 min = FLT_MAX;
		float3 max = -FLT_MAX;

		auto transXM = XMLoadFloat4x4(&GetTransformMatrix());

		for (int i = 0; i < 8; ++i)
		{
			float3 posLocal;
			posLocal.x = (i & 1) ? (_localAABB.Center.x + _localAABB.Extents.x) : (_localAABB.Center.x - _localAABB.Extents.x);
			posLocal.y = (i & 2) ? (_localAABB.Center.y + _localAABB.Extents.y) : (_localAABB.Center.y - _localAABB.Extents.y);
			posLocal.z = (i & 4) ? (_localAABB.Center.z + _localAABB.Extents.z) : (_localAABB.Center.z - _localAABB.Extents.z);

			auto posTransXM = XMVector3TransformCoord(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&posLocal)), transXM);
			float3 pos;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&pos), posTransXM);
			min = vecMin(min, pos);
			max = vecMax(max, pos);
		}

		Math::MinMaxToAxisAlignedBox(min, max, _boundsAABB);
	}

}
