#include "ToyGE\Kernel\MeshAsset.h"
#include "ToyGE\Kernel\MaterialAsset.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace ToyGE
{
	DECLARE_SHADER(, ShininessToRoughnessPS, SHADER_PS, "ShininessToRoughnessPS", "ShininessToRoughnessPS", SM_4);

	String ShininessToRoughnessTex(const String & path)
	{
		auto roughnessTexPath = path.substr(0, path.rfind('.')) + "_roughness" + TextureAsset::GetExtension();
		if (Global::GetPlatform()->FileExists(roughnessTexPath))
			return roughnessTexPath;

		auto shininessTexAsset = Asset::Find<TextureAsset>(path);
		if (!shininessTexAsset->IsInit())
		{
			shininessTexAsset->Init();

			auto roughnessTexDesc = shininessTexAsset->GetTexture()->GetDesc();
			roughnessTexDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
			roughnessTexDesc.mipLevels = 0;
			auto roughnessTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
			roughnessTex->SetDesc(roughnessTexDesc);
			roughnessTex->Init();

			auto ps = Shader::FindOrCreate<ShininessToRoughnessPS>();
			ps->SetSRV("shininessTex", shininessTexAsset->GetTexture()->GetShaderResourceView(0, 1, 0, 1));
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ roughnessTex->GetRenderTargetView(0, 0, 1) });

			auto roughnessTexAsset = std::make_shared<TextureAsset>();
			roughnessTexAsset->SetPath(roughnessTexPath);
			roughnessTexAsset->Register();
			roughnessTexAsset->Save();
		}

		return roughnessTexPath;
	}

	bool AssimpLoadMesh(const Ptr<MeshAsset> & asset, bool bFlipUV, bool bLeftHanded)
	{
		if (!Global::GetPlatform()->FileExists(asset->GetLoadPath()))
		{
			ToyGE_LOG(LT_WARNING, "Cannot open mesh file when reading! %s", asset->GetLoadPath().c_str());
			return false;
		}

		auto meshFullPath = asset->GetLoadFullPath();
		auto meshDirFullPath = ParentPath(meshFullPath);
		auto assetsFullBasePath = Global::GetPlatform()->GetPathFullName(Asset::GetAssetsBasePath());
		String meshDirRelativePath;
		if (!Global::GetPlatform()->GetRelativePath(assetsFullBasePath, true, meshDirFullPath, true, meshDirRelativePath))
		{
			ToyGE_LOG(LT_WARNING, "Cannot find relative path for mesh from %s to %s!", assetsFullBasePath.c_str(), meshDirFullPath.c_str());
			return false;
		}

		// Base path for mesh's resources
		String basePath = meshDirRelativePath;

		uint32_t processFlag = aiProcessPreset_TargetRealtime_Quality;// | aiProcess_MakeLeftHanded;
		if (bFlipUV)
			processFlag |= aiProcess_FlipUVs;
		if (bLeftHanded)
			processFlag |= aiProcess_MakeLeftHanded;

		Assimp::Importer importer;
		const aiScene *pAiScene = importer.ReadFile(asset->GetLoadPath(), processFlag);
		if (nullptr == pAiScene)
		{
			ToyGE_LOG(LT_WARNING, "Cannot load mesh! %s", asset->GetLoadPath().c_str());
			return false;
		}


		// Material
		std::vector<Ptr<MaterialAsset>> materials;

		for (uint32_t matIndex = 0; matIndex != pAiScene->mNumMaterials; ++matIndex)
		{
			Ptr<MaterialAsset> mat = std::make_shared<MaterialAsset>();
			const aiMaterial *pAiMat = pAiScene->mMaterials[matIndex];

			// Diffuse
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_DIFFUSE); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_DIFFUSE, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;

				if(basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_BASECOLOR, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_BASECOLOR, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			// Specular
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_SPECULAR); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_SPECULAR, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;
				//mat->AddTexture(MaterialTextureType::MAT_TEX_METALLIC, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), uvIndex);
				if (basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_METALLIC, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_METALLIC, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			// Normal
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_NORMALS); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_NORMALS, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;
				//mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), uvIndex);
				if (basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			// Height
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_HEIGHT); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_HEIGHT, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;
				//mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), uvIndex);
				if (basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_BUMP, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			// Roughness
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_SHININESS); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_SHININESS, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;


				if (basePath.size() > 0)
				{
					auto roughnessPath = ShininessToRoughnessTex(basePath + "/" + path.C_Str());
					mat->AddTexture(MaterialTextureType::MAT_TEX_ROUGHNESS, Asset::Find<TextureAsset>(roughnessPath), static_cast<uint8_t>(uvIndex));
				}
				else
				{
					auto roughnessPath = ShininessToRoughnessTex(path.C_Str());
					mat->AddTexture(MaterialTextureType::MAT_TEX_ROUGHNESS, Asset::Find<TextureAsset>(roughnessPath), static_cast<uint8_t>(uvIndex));
				}
			}

			// Opacity
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_OPACITY); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_OPACITY, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;
				//mat->AddTexture(MAT_TEX_OPACITYMASK, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), 0);
				if (basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_OPACITYMASK, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_OPACITYMASK, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			// EMISSIVE
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_EMISSIVE); ++texIndex)
			{
				aiString path;
				uint32_t uvIndex;
				pAiMat->GetTexture(aiTextureType_EMISSIVE, texIndex, &path, 0, &uvIndex);
				if (uvIndex >= MaxNumTexCoord::NUM)
					uvIndex = 0;
				//mat->AddTexture(MAT_TEX_EMISSIVE, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), 0);
				if (basePath.size() > 0)
					mat->AddTexture(MaterialTextureType::MAT_TEX_EMISSIVE, Asset::Find<TextureAsset>(basePath + "/" + path.C_Str()), static_cast<uint8_t>(uvIndex));
				else
					mat->AddTexture(MaterialTextureType::MAT_TEX_EMISSIVE, Asset::Find<TextureAsset>(path.C_Str()), static_cast<uint8_t>(uvIndex));
			}

			aiColor3D baseColor;
			pAiMat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
			mat->SetBaseColor(float3(baseColor.r, baseColor.g, baseColor.b));

			float opacity;
			pAiMat->Get(AI_MATKEY_OPACITY, opacity);
			mat->SetOpcacity(opacity);

			float shininess;
			pAiMat->Get(AI_MATKEY_SHININESS, shininess);
			mat->SetRoughness(pow(2.0f / (shininess + 2.0f), 0.25f));

			// Set material path
			aiString matName;
			pAiMat->Get(AI_MATKEY_NAME, matName);
			if (matName.length > 0)
			{
				if(basePath.size() > 0)
					mat->SetPath(basePath + "/Material/" + matName.C_Str() + MaterialAsset::GetExtension());
				else
					mat->SetPath(String("Material/") + matName.C_Str() + MaterialAsset::GetExtension());
			}
			else
			{
				if (basePath.size() > 0)
					mat->SetPath(basePath + "/Material/mat_" + std::to_string(matIndex) + MaterialAsset::GetExtension());
				else
					mat->SetPath(String("Material/mat_") + matName.C_Str() + MaterialAsset::GetExtension());
			}

			materials.push_back(mat);
		}


		// Mesh
		std::vector<Ptr<MeshElement>> meshElements;

		for (uint32_t meshIndex = 0; meshIndex != pAiScene->mNumMeshes; ++meshIndex)
		{
			Ptr<MeshElement> meshElement = std::make_shared<MeshElement>();
			const aiMesh *pAiMesh = pAiScene->mMeshes[meshIndex];

			meshElement->material = materials[pAiMesh->mMaterialIndex];

			auto vertexSlot = std::make_shared<MeshVertexSlotData>();
			//meshElement->vertexData = std::make_shared<MeshVertexData>();

			// Init vertex elements desc
			std::vector<MeshVertexElementDesc> & elementsDesc = vertexSlot->vertexDesc.elementsDesc;
			MeshVertexElementDesc elemDesc;
			int32_t bytesOffset = 0;
			if (pAiMesh->HasPositions())
			{
				elemDesc.signature = MeshVertexElementSignature::MVET_POSITION;
				elemDesc.signatureIndex = 0;
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			if (pAiMesh->GetNumUVChannels() > 0)
			{
				for (uint32_t uvChannel = 0; uvChannel != pAiMesh->GetNumUVChannels(); ++uvChannel)
				{
					elemDesc.signature = MeshVertexElementSignature::MVET_TEXCOORD;
					elemDesc.signatureIndex = static_cast<int32_t>(uvChannel);
					elemDesc.bytesOffset = bytesOffset;
					elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
					elementsDesc.push_back(elemDesc);
					bytesOffset += elemDesc.bytesSize;
				}
			}

			if (pAiMesh->HasNormals())
			{
				elemDesc.signature = MeshVertexElementSignature::MVET_NORMAL;
				elemDesc.signatureIndex = 0;
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			if (pAiMesh->HasTangentsAndBitangents())
			{
				elemDesc.signature = MeshVertexElementSignature::MVET_TANGENT;
				elemDesc.signatureIndex = 0;
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;

				elemDesc.signature = MeshVertexElementSignature::MVET_BITANGENT;
				elemDesc.signatureIndex = 0;
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			vertexSlot->vertexDesc.bytesSize = bytesOffset;

			// Init vertex data
			int32_t stride = bytesOffset;
			vertexSlot->bufferSize = stride * static_cast<int32_t>(pAiMesh->mNumVertices);
			vertexSlot->rawBuffer = MakeBufferedDataShared(static_cast<size_t>(vertexSlot->bufferSize));

			bytesOffset = 0;
			uint8_t * data = vertexSlot->rawBuffer.get();
			if (pAiMesh->HasPositions())
			{
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 pos;
					pos.x() = pAiMesh->mVertices[vertexIndex].x;
					pos.y() = pAiMesh->mVertices[vertexIndex].y;
					pos.z() = pAiMesh->mVertices[vertexIndex].z;
					memcpy(data, &pos, sizeof(pos));
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}

			if (pAiMesh->GetNumUVChannels() > 0)
			{
				data = vertexSlot->rawBuffer.get() + bytesOffset;
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					for (uint32_t uvChannel = 0; uvChannel != pAiMesh->GetNumUVChannels(); ++uvChannel)
					{
						auto & uvw = pAiMesh->mTextureCoords[uvChannel][vertexIndex];
						float3 texCoord(uvw.x, uvw.y, uvw.z);
						memcpy(data + sizeof(texCoord) * uvChannel, &texCoord, sizeof(texCoord));
					}
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3) * pAiMesh->GetNumUVChannels());
			}

			if (pAiMesh->HasNormals())
			{
				data = vertexSlot->rawBuffer.get() + bytesOffset;
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 normal;
					normal.x() = pAiMesh->mNormals[vertexIndex].x;
					normal.y() = pAiMesh->mNormals[vertexIndex].y;
					normal.z() = pAiMesh->mNormals[vertexIndex].z;
					normal = normalize(normal);
					memcpy(data, &normal, sizeof(normal));
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}

			if (pAiMesh->HasTangentsAndBitangents())
			{
				data = vertexSlot->rawBuffer.get() + bytesOffset;
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 tangent;
					tangent.x() = pAiMesh->mTangents[vertexIndex].x;
					tangent.y() = pAiMesh->mTangents[vertexIndex].y;
					tangent.z() = pAiMesh->mTangents[vertexIndex].z;
					memcpy(data, &tangent, sizeof(tangent));

					float3 bitangent;
					bitangent.x() = pAiMesh->mBitangents[vertexIndex].x;
					bitangent.y() = pAiMesh->mBitangents[vertexIndex].y;
					bitangent.z() = pAiMesh->mBitangents[vertexIndex].z;
					memcpy(data + sizeof(tangent), &bitangent, sizeof(bitangent));

					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}
			meshElement->vertexData.push_back(vertexSlot);

			// Indices
			for (uint32_t faceIndex = 0; faceIndex != pAiMesh->mNumFaces; ++faceIndex)
			{
				aiFace &pAiFace = pAiMesh->mFaces[faceIndex];
				for (uint32_t index = 0; index != pAiFace.mNumIndices; ++index)
					meshElement->indices.push_back(pAiFace.mIndices[index]);
			}

			meshElements.push_back(meshElement);
		}
		asset->SetData(meshElements);

		return true;
	}

	void MeshAsset::Load()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to load mesh with empty path!");
			return;
		}

		auto extension = _path.substr(_path.rfind('.'));
		if (extension != MeshAsset::GetExtension())
		{
			if (!AssimpLoadMesh(shared_from_this()->Cast<MeshAsset>(), false, false))
				return;
		}
		else
		{
			auto file = Global::GetPlatform()->CreatePlatformFile(GetLoadPath(), FILE_OPEN_READ);
			if (!file->IsValid())
			{
				ToyGE_LOG(LT_WARNING, "Cannot open mesh file when reading! %s", GetLoadPath().c_str());
				return;
			}
			auto reader = std::make_shared<FileReader>(file);

			_data.resize(reader->Read<int32_t>());
			for (auto & element : _data)
			{
				// vertex data
				element = std::make_shared<MeshElement>();

				// vertex slots
				element->vertexData.resize(reader->Read<int32_t>());
				for (auto & vertexSlot : element->vertexData)
				{
					vertexSlot = std::make_shared<MeshVertexSlotData>();
					vertexSlot->bufferSize = reader->Read<int32_t>();
					vertexSlot->rawBuffer = MakeBufferedDataShared(vertexSlot->bufferSize);
					reader->ReadBytes(vertexSlot->rawBuffer.get(), vertexSlot->bufferSize);
					vertexSlot->vertexDesc.bytesSize = reader->Read<int32_t>();
					vertexSlot->vertexDesc.elementsDesc.resize(reader->Read<int32_t>());
					reader->ReadBytes(&vertexSlot->vertexDesc.elementsDesc[0], sizeof(MeshVertexElementDesc) * vertexSlot->vertexDesc.elementsDesc.size());
				}

				// indices
				element->indices.resize(reader->Read<int32_t>());
				reader->ReadBytes(&element->indices[0], sizeof(uint32_t) * element->indices.size());

				// material
				String matRelativePath;
				reader->ReadString(matRelativePath);
				if (matRelativePath.size() > 0)
					element->material = Asset::Find<MaterialAsset>( IdenticalPath(GetPath() + "/../" + matRelativePath) );
			}
		}
		_bLoaded = true;
		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Mesh loaded! %s", GetLoadPath().c_str());
	}

	void MeshAsset::Init()
	{
		for (auto & meshElement : _data)
		{
			if (!meshElement->material->IsInit())
				meshElement->material->Init();
		}

		_mesh = std::make_shared<Mesh>();
		_mesh->SetAsset(shared_from_this()->Cast<MeshAsset>());
		_mesh->SetData(_data);
		_mesh->Init();

		for (auto & meshElementRender : _mesh->GetRenderData()->GetMeshElements())
		{
			for(auto & vb : meshElementRender->GetVertexBuffer())
				vb->SetDirty(false);
			meshElementRender->GetIndexBuffer()->SetDirty(false);
			meshElementRender->SetDirty(false);
		}
		_mesh->SetDirty(false);

		_bInit = true;
	}

	void MeshAsset::Save()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to save mesh with empty path!");
			return;
		}

		// Save material assets
		for (auto & meshElement : _data)
		{
			if(meshElement->material->IsDirty())
				meshElement->material->Save();
		}

		// Update if necessary
		if (_mesh && _mesh->IsDirty())
		{
			UpdateFromMesh();
		}

		MakePathDirectorys(ParentPath(GetSavePath()));
		auto file = Global::GetPlatform()->CreatePlatformFile(GetSavePath(), FILE_OPEN_WRITE);
		if (!file->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open mesh file when saving! %s", file->GetPath().c_str());
			return;
		}
		auto writer = std::make_shared<FileWriter>(file);

		writer->Write<int32_t>(static_cast<int32_t>(_data.size()));
		for (auto & element : _data)
		{
			// vertex data
			writer->Write<int32_t>(static_cast<int32_t>(element->vertexData.size()));
			for (auto & vertexSlot : element->vertexData)
			{
				writer->Write<int32_t>(static_cast<int32_t>(vertexSlot->bufferSize));
				writer->WriteBytes(vertexSlot->rawBuffer.get(), vertexSlot->bufferSize);
				writer->Write<int32_t>(vertexSlot->vertexDesc.bytesSize);
				writer->Write<int32_t>(static_cast<int32_t>(vertexSlot->vertexDesc.elementsDesc.size()));
				writer->WriteBytes(&vertexSlot->vertexDesc.elementsDesc[0], sizeof(MeshVertexElementDesc) * vertexSlot->vertexDesc.elementsDesc.size());
			}

			// indices
			writer->Write<int32_t>(static_cast<int32_t>(element->indices.size()));
			writer->WriteBytes(&element->indices[0], sizeof(uint32_t) * element->indices.size());

			// material
			if (element->material)
			{
				String matRelatvePath;
				Global::GetPlatform()->GetRelativePath(
					ParentPath(GetLoadFullPath()), true,
					element->material->GetLoadFullPath(), false,
					matRelatvePath);
				writer->WriteString(matRelatvePath);
			}
			else
				writer->WriteString("");
		}

		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Mesh saved! %s", file->GetPath().c_str());
	}

	void MeshAsset::UpdateFromMesh()
	{
		GetMesh()->UpdateFromRenderData();
		_data = GetMesh()->GetData();
	}
}