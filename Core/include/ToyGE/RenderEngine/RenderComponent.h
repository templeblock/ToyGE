#pragma once
#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Mesh.h"

namespace ToyGE
{
	class ReflectionMap;
	class Texture;
	class Mesh;
	class Material;

	class TOYGE_CORE_API RenderComponent : public TransformComponent, public Cullable
	{
	public:
		RenderComponent();

		virtual ~RenderComponent() = default;

		void SetRenderMeshComponent(const Ptr<class RenderMeshComponent> & com)
		{
			_renderMeshComponent = com;
		}
		Ptr<class RenderMeshComponent> GetRenderMeshComponent() const
		{
			return _renderMeshComponent.lock();
		}

		void SetMeshElement(const Ptr<MeshElementRenderData> & meshElement)
		{
			if (_meshElement != meshElement)
			{
				_meshElement = meshElement;
				UpdateLocalAABB();
				UpdateBoundsAABB();
			}
		}

		const Ptr<MeshElementRenderData> & GetMeshElement() const
		{
			return _meshElement;
		}

		void SetMaterial(const Ptr<Material> & material)
		{
			_material = material;
		}

		const Ptr<Material> & GetMaterial() const
		{
			return _material;
		}

		const AABBox & GetLocalAABB() const
		{
			return _localAABB;
		}

		virtual AABBox GetBoundsAABB() const override;

		void SetCastShadows(bool bCastShadows)
		{
			_bCastShadows = bCastShadows;
		}

		bool IsCastShadows() const
		{
			return _bCastShadows;
		}

		void SetCastCaustics(bool bCastCaustics)
		{
			_bCastCaustics = bCastCaustics;
		}

		bool IsCastCaustics() const
		{
			return _bCastCaustics;
		}

		virtual void Activate() override;

		bool IsSpecialRender() const
		{
			return _bSpecialRender;
		}

		virtual void SpecialRender(const Ptr<class RenderView> & view) {}

	protected:
		std::weak_ptr<class RenderMeshComponent> _renderMeshComponent;
		Ptr<MeshElementRenderData> _meshElement;
		Ptr<Material> _material;
		AABBox _localAABB;
		AABBox _boundsAABB;
		bool _bCastShadows;
		bool _bCastCaustics;
		bool _bSpecialRender;

		void OnTranformUpdated();

	private:
		void UpdateLocalAABB();
		void UpdateBoundsAABB();
	};

	class TOYGE_CORE_API RenderMeshComponent : public TransformComponent
	{
	public:
		RenderMeshComponent();

		virtual ~RenderMeshComponent() = default;

		void SetMesh(const Ptr<Mesh> & mesh);

		const Ptr<Mesh> & GetMesh() const
		{
			return _mesh;
		}

		CLASS_GET(SubRenderComponents, std::vector<Ptr<RenderComponent>>, _renderComponents);

		virtual void Activate() override;

	protected:
		Ptr<Mesh> _mesh;
		std::vector<Ptr<RenderComponent>> _renderComponents;

		void OnTranformUpdated();
	};
}

#endif