#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class Mesh;
	class Material;
	class RenderComponent;
	class File;
	class Scene;

	class TOYGE_CORE_API Model
	{
		friend TOYGE_CORE_API bool LoadModel(const Ptr<File> & file, const Ptr<Model> & model, bool);

	public:
		static Ptr<Model> Load(const Ptr<File> & file);

		void SaveBin(const Ptr<Writer> & writer);

		static Ptr<Model> LoadBin(const Ptr<Reader> & reader, const WString & path);

		void AddMaterial(const Ptr<Material> & mat)
		{
			_materials.push_back(mat);
		}

		int32_t NumMaterials() const
		{
			return static_cast<int32_t>(_materials.size());
		}

		const Ptr<Material> & GetMaterial(int32_t index) const
		{
			return _materials[index];
		}

		void AddMesh(const Ptr<Mesh> & mesh)
		{
			_meshes.push_back(mesh);
		}

		int32_t NumMeshes() const
		{
			return static_cast<int32_t>(_meshes.size());
		}

		const Ptr<Mesh> & GetMesh(int32_t index) const
		{
			return _meshes[index];
		}

		void SetPath(const WString & path)
		{
			_path = path;
		}

		const WString & Path() const
		{
			return _path;
		}

		void RegisterResources() const;

		void InitRenderData();

		void AddInstanceToScene(
			const Ptr<Scene> & scene,
			const XMFLOAT3 & pos = XMFLOAT3(0.0f, 0.0f, 0.0f),
			const XMFLOAT3 & scale = XMFLOAT3(1.0f, 1.0f, 1.0f),
			const XMFLOAT4 & orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
			std::vector<Ptr<RenderComponent>> * outComponents = nullptr);

	private:
		std::vector<Ptr<Material>> _materials;
		std::vector<Ptr<Mesh>> _meshes;
		std::map<int32_t, int32_t> _meshMaterialsMap;
		WString _path;
	};
}

#endif