#pragma once
#ifndef RENDERCOMPONENT_H
#define RENDERCOMPONENT_H

#include "ToyGE\RenderEngine\TransformComponent.h"
#include "ToyGE\RenderEngine\SceneCuller.h"

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


		void SetMesh(const Ptr<Mesh> & mesh)
		{
			if (_mesh != mesh)
			{
				_mesh = mesh;
				UpdateLocalAABB();
				UpdateBoundsAABB();
			}
			else
			{
				_mesh = mesh;
			}
		}

		const Ptr<Mesh> & GetMesh() const
		{
			return _mesh;
		}

		void SetMaterial(const Ptr<Material> & material)
		{
			_material = material;
		}

		const Ptr<Material> & GetMaterial() const
		{
			return _material;
		}

		const XNA::AxisAlignedBox & GetLocalAABB() const
		{
			return _localAABB;
		}

		XNA::AxisAlignedBox GetBoundsAABB() const override;

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
		Ptr<Mesh> _mesh;
		Ptr<Material> _material;
		XNA::AxisAlignedBox _localAABB;
		XNA::AxisAlignedBox _boundsAABB;
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
}

#endif