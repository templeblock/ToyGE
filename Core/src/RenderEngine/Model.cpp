#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\RenderEngine\ModelLoader.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\SceneObject.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"

namespace ToyGE
{
	Ptr<Model> Model::Load(const Ptr<File> & file)
	{
		ToyGE_ASSERT(file);
		if (!file->CanRead())
			return Ptr<Model>();

		Ptr<Model> model = std::make_shared<Model>();
		if (LoadModel(file, model, false))
			return model;
		else
			return Ptr<Model>();
	}

	void Model::SaveBin(const Ptr<Writer> & writer)
	{
		std::map<Ptr<Material>, int32_t> materialMap;
		writer->Write<int32_t>(static_cast<int32_t>(_materials.size()));
		int matIndex = 0;
		auto matBasePath = _path.substr(0, _path.find_last_of(L'\\') + 1);
		for (auto & mat : _materials)
		{
			mat->SaveBin(writer, matBasePath);
			materialMap[mat] = matIndex;
			++matIndex;
		}

		writer->Write<int32_t>(static_cast<int32_t>(_meshes.size()));
		for (auto & mesh : _meshes)
		{
			mesh->SaveBin(writer, materialMap);
		}

		writer->Write<int32_t>(static_cast<int32_t>(_meshMaterialsMap.size()));
		for (auto & i : _meshMaterialsMap)
		{
			writer->Write<int32_t>(i.first);
			writer->Write<int32_t>(i.second);
		}
	}

	Ptr<Model> Model::LoadBin(const Ptr<Reader> & reader, const WString & path)
	{
		auto model = std::make_shared<Model>();
		model->_path = path;
		auto matBasePath = path.substr(0, path.find_last_of('\\') + 1);
		int32_t numMaterials = reader->Read<int32_t>();
		for (int32_t i = 0; i < numMaterials; ++i)
		{
			model->_materials.push_back(Material::LoadBin(reader, matBasePath));
		}

		int32_t numMeshes = reader->Read<int32_t>();
		for (int32_t i = 0; i < numMeshes; ++i)
		{
			model->_meshes.push_back(Mesh::LoadBin(reader, model->_materials));
		}

		int32_t meshMaterialMapSize = reader->Read<int32_t>();
		for (int32_t i = 0; i < meshMaterialMapSize; ++i)
		{
			auto key = reader->Read<int32_t>();
			auto value = reader->Read<int32_t>();
			model->_meshMaterialsMap.insert(std::make_pair(key, value));
		}

		return model;
	}

	void Model::RegisterResources() const
	{
		for (auto & mat : _materials)
		{
			Global::GetResourceManager(RESOURCE_MATERIAL)->As<MaterialManager>()->SetResource(mat, mat->Name());
		}
		for (auto & mesh : _meshes)
		{
			Global::GetResourceManager(RESOURCE_MESH)->As<MeshManager>()->SetResource(mesh, mesh->Name());
		}
	}

	void Model::InitRenderData()
	{
		for (auto & mat : _materials)
		{
			mat->InitRenderData();
		}
		for (auto & mesh : _meshes)
		{
			mesh->InitRenderData();
		}
	}

	void Model::AddInstanceToScene(
		const Ptr<Scene> & scene,
		const XMFLOAT3 & pos,
		const XMFLOAT3 & scale,
		const XMFLOAT4 & orientation,
		std::vector<Ptr<RenderComponent>> * outComponents)
	{
		int32_t meshIndex = 0;
		for (auto & mesh : _meshes)
		{
			auto obj = mesh->AddInstanceToScene(scene, pos, scale, orientation);
			auto materialFind = _meshMaterialsMap.find(meshIndex);
			if (materialFind != _meshMaterialsMap.end())
			{
				obj->SetMaterial(_materials[materialFind->second]);
			}

			if (outComponents != nullptr)
			{
				outComponents->push_back(obj);
			}

			++meshIndex;
		}
	}
}