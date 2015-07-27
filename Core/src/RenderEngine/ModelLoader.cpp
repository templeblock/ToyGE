#include "ToyGE\RenderEngine\ModelLoader.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\Util.h"

#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

namespace ToyGE
{
	bool LoadModel(const Ptr<File> & file, const Ptr<Model> & model, bool bFlipUV)
	{
		ToyGE_ASSERT(file);
		ToyGE_ASSERT(model);
		if (!file->CanRead())
			return false;

		String filePath;
		ConvertStr_WToA(file->Path(), filePath);

		uint32_t processFlag = aiProcessPreset_TargetRealtime_Quality | aiProcess_ConvertToLeftHanded;

		if (bFlipUV)
			processFlag |= aiProcess_FlipUVs;

		Assimp::Importer importer;
		const aiScene *pAiScene = importer.ReadFile(filePath, processFlag);
		//::OutputDebugStringA(importer.GetErrorString());
		if (nullptr == pAiScene)
			return false;

		model->SetPath(file->Path());

		WString basePath = file->ParentPath();

		for (uint32_t matIndex = 0; matIndex != pAiScene->mNumMaterials; ++matIndex)
		{
			Ptr<Material> mat = std::make_shared<Material>();
			const aiMaterial *pAiMat = pAiScene->mMaterials[matIndex];

			//Diffuse
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_DIFFUSE); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_BASECOLOR, basePath + wPath, 0);
			}

			//Specular
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_SPECULAR); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_SPECULAR, basePath + wPath, 0);
			}

			//Normal
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_NORMALS); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_NORMALS, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_NORMAL, basePath + wPath, 0);
			}

			//Height
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_HEIGHT); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_HEIGHT, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_HEIGHT, basePath + wPath, 0);
			}

			//Shininess
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_SHININESS); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_SHININESS, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_SHININESS, basePath + wPath, 0);
			}

			//Opacity
			for (uint32_t texIndex = 0; texIndex != pAiMat->GetTextureCount(aiTextureType_OPACITY); ++texIndex)
			{
				aiString path;
				pAiMat->GetTexture(aiTextureType_OPACITY, texIndex, &path);
				WString wPath;
				ConvertStr_AToW(path.C_Str(), wPath);
				mat->AddTexture(MATERIAL_TEXTURE_OPACITYMASK, basePath + wPath, 0);
			}

			aiColor3D color;
			float fp;

			pAiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			pAiMat->Get(AI_MATKEY_OPACITY, fp);
			mat->SetBaseColor(float3(color.r, color.g, color.b));

			//pAiMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
			//mat->SetMetallic(XMFLOAT3(color.r, color.g, color.b));

			pAiMat->Get(AI_MATKEY_SHININESS, fp);
			mat->SetRoughness(pow(2.0f / (fp + 2.0f), 0.25f));

			aiString matName;
			pAiMat->Get(AI_MATKEY_NAME, matName);
			WString wMatName;
			ConvertStr_AToW(matName.C_Str(), wMatName);
			mat->SetName(file->Path() + L"\\" + wMatName);
			
			model->AddMaterial(mat);
		}

		
		for (uint32_t meshIndex = 0; meshIndex != pAiScene->mNumMeshes; ++meshIndex)
		{
			Ptr<Mesh> mesh = std::make_shared<Mesh>();
			const aiMesh *pAiMesh = pAiScene->mMeshes[meshIndex];

			WString wMeshName;
			ConvertStr_AToW(pAiMesh->mName.C_Str(), wMeshName);

			mesh->SetName(file->Path() + L"\\" + wMeshName);
			model->_meshMaterialsMap[static_cast<int32_t>(meshIndex)] = static_cast<int32_t>(pAiMesh->mMaterialIndex);

			VertexDataDesc vertexDataDesc;

			//Init Elements Desc
			std::vector<VertexElementDesc> & elementsDesc = vertexDataDesc.elementsDesc;
			VertexElementDesc elemDesc;
			int32_t bytesOffset = 0;

			if (pAiMesh->HasPositions())
			{
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elemDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
				elemDesc.index = 0;
				elemDesc.instanceDataRate = 0;
				elemDesc.name = StandardVertexElementName::Position();
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			if (pAiMesh->GetNumUVChannels() > 0)
			{
				for (uint32_t uvChannel = 0; uvChannel != pAiMesh->GetNumUVChannels(); ++uvChannel)
				{
					elemDesc.bytesOffset = bytesOffset;
					elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
					elemDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
					elemDesc.index = uvChannel;
					elemDesc.instanceDataRate = 0;
					elemDesc.name = StandardVertexElementName::TextureCoord();
					elementsDesc.push_back(elemDesc);
					bytesOffset += elemDesc.bytesSize;
				}
			}

			if (pAiMesh->HasNormals())
			{
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elemDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
				elemDesc.index = 0;
				elemDesc.instanceDataRate = 0;
				elemDesc.name = StandardVertexElementName::Normal();
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			if (pAiMesh->HasTangentsAndBitangents())
			{
				elemDesc.bytesOffset = bytesOffset;
				elemDesc.bytesSize = static_cast<int32_t>(sizeof(float3));
				elemDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
				elemDesc.index = 0;
				elemDesc.instanceDataRate = 0;
				elemDesc.name = StandardVertexElementName::Tangent();
				elementsDesc.push_back(elemDesc);
				bytesOffset += elemDesc.bytesSize;
			}

			vertexDataDesc.vertexByteSize = bytesOffset;

			//Init Datas
			vertexDataDesc.numVertices = static_cast<int32_t>(pAiMesh->mNumVertices);
			int32_t stride = bytesOffset;
			auto dataBuffer = MakeBufferedDataShared(static_cast<size_t>(stride)* pAiMesh->mNumVertices);

			bytesOffset = 0;
			uint8_t * data = dataBuffer.get();
			//Pos
			if (pAiMesh->HasPositions())
			{
				//mesh->AddDataFlag(MESH_DATA_POS);
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 pos;
					pos.x = pAiMesh->mVertices[vertexIndex].x;
					pos.y = pAiMesh->mVertices[vertexIndex].y;
					pos.z = pAiMesh->mVertices[vertexIndex].z;
					memcpy(data, &pos, sizeof(pos));
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}

			//UV
			if (pAiMesh->GetNumUVChannels() > 0)
			{
				data = dataBuffer.get() + bytesOffset;
				//mesh->AddDataFlag(MESH_DATA_UV);
				/*mesh->SetNumUVChannels(pAiMesh->GetNumUVChannels());
				for (uint32_t uvChannel = 0; uvChannel != pAiMesh->GetNumUVChannels(); ++uvChannel)
				{
					mesh->SetUVChannelNumComponents(uvChannel, static_cast<uint8_t>(pAiMesh->mNumUVComponents[uvChannel]) );
				}*/

				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					//StandardVertex & vertex = vertices[vertexIndex];
					for (uint32_t uvChannel = 0; uvChannel != pAiMesh->GetNumUVChannels(); ++uvChannel)
					{
						auto & uvw = pAiMesh->mTextureCoords[uvChannel][vertexIndex];
						float3 texCoord(uvw.x, uvw.y, uvw.z);
						//vertex.texCoord[uvChannel] = float3(uv.x, uv.y, uv.z);
						memcpy(data + sizeof(texCoord) * uvChannel, &texCoord, sizeof(texCoord));
					}
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3) * pAiMesh->GetNumUVChannels());
			}

			//Normals
			if (pAiMesh->HasNormals())
			{
				data = dataBuffer.get() + bytesOffset;
				//mesh->AddDataFlag(MESH_DATA_NORMAL);
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 normal;
					normal.x = pAiMesh->mNormals[vertexIndex].x;
					normal.y = pAiMesh->mNormals[vertexIndex].y;
					normal.z = pAiMesh->mNormals[vertexIndex].z;
					normal = normalize(normal);
					memcpy(data, &normal, sizeof(normal));
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}

			//Tangents
			if (pAiMesh->HasTangentsAndBitangents())
			{
				data = dataBuffer.get() + bytesOffset;
				//mesh->AddDataFlag(MESH_DATA_TANGENT);
				for (uint32_t vertexIndex = 0; vertexIndex != pAiMesh->mNumVertices; ++vertexIndex)
				{
					float3 tangent;
					tangent.x = pAiMesh->mTangents[vertexIndex].x;
					tangent.y = pAiMesh->mTangents[vertexIndex].y;
					tangent.z = pAiMesh->mTangents[vertexIndex].z;
					memcpy(data, &tangent, sizeof(tangent));
					data += stride;
				}
				bytesOffset += static_cast<int32_t>(sizeof(float3));
			}

			vertexDataDesc.pData = dataBuffer;

			mesh->AddVertexData(vertexDataDesc);

			for (uint32_t faceIndex = 0; faceIndex != pAiMesh->mNumFaces; ++faceIndex)
			{
				aiFace &pAiFace = pAiMesh->mFaces[faceIndex];
				for (uint32_t index = 0; index != pAiFace.mNumIndices; ++index)
					mesh->AddIndex(pAiFace.mIndices[index]);
			}

			model->AddMesh(mesh);
		}

		return true;
	}
}