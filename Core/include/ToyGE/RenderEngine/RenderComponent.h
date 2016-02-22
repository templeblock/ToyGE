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

	class TOYGE_CORE_API RenderComponent : public TransformComponent, public Cullable, public std::enable_shared_from_this<RenderComponent>
	{
	public:
		static String Name();

		RenderComponent();

		String GetComponentName() const override;

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

		AABBox GetBoundsAABB() const override;

		void SetReflectionMap(const Ptr<ReflectionMap> & reflectionMap)
		{
			_reflectionMap = reflectionMap;
		}

		const Ptr<ReflectionMap> & GetReflectionMap() const
		{
			return _reflectionMap;
		}

		void SetEnvMap(const Ptr<Texture> & envMap)
		{
			_envMap = envMap;
		}

		const Ptr<Texture> & GetEnvMap() const
		{
			return _envMap;
		}

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

	protected:
		std::weak_ptr<class RenderMeshComponent> _renderMeshComponent;
		Ptr<MeshElementRenderData> _meshElement;
		Ptr<Material> _material;
		AABBox _localAABB;
		AABBox _boundsAABB;
		Ptr<ReflectionMap> _reflectionMap;
		Ptr<Texture> _envMap;
		bool _bCastShadows;
		bool _bCastCaustics;

		void DoActive() override;

		void OnTranformUpdated() override;

	private:
		void UpdateLocalAABB();
		void UpdateBoundsAABB();
	};

	class TOYGE_CORE_API RenderMeshComponent : public TransformComponent, public std::enable_shared_from_this<RenderMeshComponent>
	{
	public:
		static String Name()
		{
			return "";
		}

		String GetComponentName() const override
		{
			return "";
		}

		void SetMesh(const Ptr<Mesh> & mesh);

		const Ptr<Mesh> & GetMesh() const
		{
			return _mesh;
		}

		CLASS_GET(SubRenderComponents, std::vector<Ptr<RenderComponent>>, _renderComponents);

	protected:
		Ptr<Mesh> _mesh;
		std::vector<Ptr<RenderComponent>> _renderComponents;

		void DoActive() override;

		void OnTranformUpdated() override;
	};
}

#endif