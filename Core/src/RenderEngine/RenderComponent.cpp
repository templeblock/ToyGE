#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Mesh.h"

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

	void GetMeshElementAABB(const Ptr<MeshElement> & meshElement, XNA::AxisAlignedBox & outAABB)
	{
		Ptr<MeshVertexSlotData> positionVertexSlot;
		for (auto & vertexSlot : meshElement->vertexData)
		{
			if (vertexSlot->FindVertexElement(MeshVertexElementSignature::MVET_POSITION, 0) >= 0)
				positionVertexSlot = vertexSlot;
		}

		if (positionVertexSlot)
		{
			XNA::ComputeBoundingAxisAlignedBoxFromPoints(
				&outAABB,
				static_cast<UINT>(positionVertexSlot->GetNumVertices()),
				positionVertexSlot->GetElement<XMFLOAT3>(0, MeshVertexElementSignature::MVET_POSITION, 0),
				static_cast<UINT>(positionVertexSlot->vertexDesc.bytesSize));
		}
		else
			Math::MinMaxToAxisAlignedBox(1.0f, -1.0f, outAABB);
	}

	void RenderComponent::UpdateLocalAABB()
	{
		if (!_meshElement)
			_localAABB.Extents = XMFLOAT3(-1.0f, -1.0f, -1.0f);
		else
		{
			XNA::AxisAlignedBox aabb;
			
			GetMeshElementAABB(_meshElement->GetElementData(), _localAABB);
		}
	}

	void RenderComponent::UpdateBoundsAABB()
	{
		float3 min = FLT_MAX;
		float3 max = -FLT_MAX;

		auto transXM = XMLoadFloat4x4(&GetWorldTransformMatrix());

		for (int i = 0; i < 8; ++i)
		{
			float3 posLocal;
			posLocal.x() = (i & 1) ? (_localAABB.Center.x + _localAABB.Extents.x) : (_localAABB.Center.x - _localAABB.Extents.x);
			posLocal.y() = (i & 2) ? (_localAABB.Center.y + _localAABB.Extents.y) : (_localAABB.Center.y - _localAABB.Extents.y);
			posLocal.z() = (i & 4) ? (_localAABB.Center.z + _localAABB.Extents.z) : (_localAABB.Center.z - _localAABB.Extents.z);

			auto posTransXM = XMVector3TransformCoord(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&posLocal)), transXM);
			float3 pos;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&pos), posTransXM);
			min = min_vec(min, pos);
			max = max_vec(max, pos);
		}

		Math::MinMaxToAxisAlignedBox(min, max, _boundsAABB);
	}


	void RenderMeshComponent::SetMesh(const Ptr<Mesh> & mesh)
	{
		if (_mesh != mesh)
		{
			_mesh = mesh;

			_renderComponents.clear();
			for (auto & meshElement : mesh->GetRenderData()->GetMeshElements())
			{
				auto renderCom = std::make_shared<RenderComponent>();
				renderCom->SetRenderMeshComponent(shared_from_this());
				renderCom->SetMeshElement(meshElement);
				renderCom->AttachTo(shared_from_this());
				_renderComponents.push_back(renderCom);
			}
		}
	}

	void RenderMeshComponent::DoActive()
	{
		for (auto & com : _renderComponents)
			com->Active();
	}

	void RenderMeshComponent::OnTranformUpdated()
	{
		for (auto & com : _renderComponents)
			com->UpdateTransform();
	}
}
