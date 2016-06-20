#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Mesh.h"

namespace ToyGE
{
	RenderComponent::RenderComponent()
		: _bCastShadows(true),
		_bCastCaustics(true),
		_bSpecialRender(false)
	{
		memset(&_localAABB, 0, sizeof(_localAABB));
		memset(&_boundsAABB, 0, sizeof(_boundsAABB));

		OnTransformChanged().connect(std::bind(&RenderComponent::OnTranformUpdated, this));
	}

	AABBox RenderComponent::GetBoundsAABB() const
	{
		return _boundsAABB;
	}

	void RenderComponent::Activate()
	{
		TransformComponent::Activate();

		auto renderObjCuller = Global::GetRenderEngine()->GetSceneRenderObjsCuller();
		renderObjCuller->AddElement(Cast<RenderComponent>());
	}

	void RenderComponent::OnTranformUpdated()
	{
		UpdateBoundsAABB();
		if (IsActive())
		{
			Global::GetRenderEngine()->GetSceneRenderObjsCuller()->UpdateElement(Cast<RenderComponent>());
		}
	}

	void GetMeshElementAABB(const Ptr<MeshElement> & meshElement, AABBox & outAABB)
	{
		Ptr<MeshVertexSlotData> positionVertexSlot;
		for (auto & vertexSlot : meshElement->vertexData)
		{
			if (vertexSlot->FindVertexElement(MeshVertexElementSignature::MVET_POSITION, 0) >= 0)
				positionVertexSlot = vertexSlot;
		}

		if (positionVertexSlot)
		{
			VertexBufferIterator<float3> begin(
				positionVertexSlot->GetElement<float3>(0, MeshVertexElementSignature::MVET_POSITION, 0), 
				positionVertexSlot->vertexDesc.bytesSize);
			VertexBufferIterator<float3> end(
				positionVertexSlot->GetElement<float3>(positionVertexSlot->GetNumVertices(), MeshVertexElementSignature::MVET_POSITION, 0),
				positionVertexSlot->vertexDesc.bytesSize);
			outAABB = compute_aabbox(begin, end);
		}
		else
			outAABB = AABBox(1.0f, 1.0f);
	}

	void RenderComponent::UpdateLocalAABB()
	{
		if (!_meshElement)
		{
			_localAABB.min = 1.0f;
			_localAABB.max = -1.0f;
		}
		else
		{
			GetMeshElementAABB(_meshElement->GetElementData(), _localAABB);
		}
	}

	void RenderComponent::UpdateBoundsAABB()
	{
		_boundsAABB = transform_aabb(_localAABB, GetWorldTransformMatrix());
	}


	RenderMeshComponent::RenderMeshComponent()
	{
		OnTransformChanged().connect(std::bind(&RenderMeshComponent::OnTranformUpdated, this));
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
				renderCom->SetRenderMeshComponent(Cast<RenderMeshComponent>());
				renderCom->SetMeshElement(meshElement);
				renderCom->AttachTo(Cast<RenderMeshComponent>());
				_renderComponents.push_back(renderCom);
			}
		}
	}

	void RenderMeshComponent::Activate()
	{
		TransformComponent::Activate();

		for (auto & com : _renderComponents)
			com->Activate();
	}

	void RenderMeshComponent::OnTranformUpdated()
	{
		for (auto & com : _renderComponents)
			com->UpdateTransform();
	}
}
