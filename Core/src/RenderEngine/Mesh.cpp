#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\StaticMeshActor.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderContext.h"

namespace ToyGE
{
	void Mesh::Init()
	{
		_renderData = std::make_shared<MeshRenderData>();
		for (auto & assetMeshElement : _data)
		{
			auto meshElementRender = std::make_shared<MeshElementRenderData>();
			meshElementRender->_meshElementData = assetMeshElement;

			RenderBufferDesc bufDesc;
			bufDesc.bindFlag = BUFFER_BIND_VERTEX;
			bufDesc.cpuAccess = 0;
			bufDesc.bStructured = false;

			// Vertex buffer
			meshElementRender->_vertexBuffers.resize(assetMeshElement->vertexData.size());
			int32_t slotIndex = 0;
			for (auto & vb : meshElementRender->_vertexBuffers)
			{
				vb = Global::GetRenderEngine()->GetRenderFactory()->CreateVertexBuffer();
				bufDesc.elementSize = assetMeshElement->vertexData[slotIndex]->vertexDesc.bytesSize;
				bufDesc.numElements = assetMeshElement->vertexData[slotIndex]->GetNumVertices();
				vb->SetDesc(bufDesc);
				vb->SetType(VertexBufferType::VERTEX_BUFFER_GEOMETRY);
				vb->Init(assetMeshElement->vertexData[slotIndex]->rawBuffer.get());

				// Vertex elements desc
				std::vector<VertexElementDesc> vertexElementDescList;
				VertexElementDesc vertexElementDesc;
				vertexElementDesc.instanceDataRate = 0;
				for (auto & assetVertexElementDesc : assetMeshElement->vertexData[slotIndex]->vertexDesc.elementsDesc)
				{
					switch (assetVertexElementDesc.signature)
					{
					case MeshVertexElementSignature::MVET_POSITION:
						vertexElementDesc.name = "POSITION";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					case MeshVertexElementSignature::MVET_TEXCOORD:
						vertexElementDesc.name = "TEXCOORD";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					case MeshVertexElementSignature::MVET_NORMAL:
						vertexElementDesc.name = "NORMAL";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					case MeshVertexElementSignature::MVET_TANGENT:
						vertexElementDesc.name = "TANGENT";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					case MeshVertexElementSignature::MVET_BITANGENT:
						vertexElementDesc.name = "BITANGENT";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					case MeshVertexElementSignature::MVET_COLOR:
						vertexElementDesc.name = "COLOR";
						vertexElementDesc.format = RENDER_FORMAT_R32G32B32_FLOAT;
						break;
					default:
						break;
					}
					vertexElementDesc.index = assetVertexElementDesc.signatureIndex;
					vertexElementDesc.bytesOffset = assetVertexElementDesc.bytesOffset;
					vertexElementDesc.bytesSize = assetVertexElementDesc.bytesSize;

					vertexElementDescList.push_back(vertexElementDesc);
				}
				vb->SetElementsDesc(vertexElementDescList);

				++slotIndex;
			}

			// Indices
			meshElementRender->_indexBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
			bufDesc.bindFlag = BUFFER_BIND_INDEX;
			bufDesc.cpuAccess = 0;
			bufDesc.elementSize = sizeof(uint32_t);
			bufDesc.numElements = static_cast<int32_t>( assetMeshElement->indices.size() );
			meshElementRender->_indexBuffer->SetDesc(bufDesc);
			meshElementRender->_indexBuffer->Init(&assetMeshElement->indices[0]);

			// Material
			if(assetMeshElement->material)
				meshElementRender->_material = assetMeshElement->material->GetMaterial();

			_renderData->_meshElements.push_back(meshElementRender);
		}
		
		InitDepthRenderData();
	}

	void Mesh::InitDepthRenderData()
	{
		int32_t index = 0;
		for (auto & meshElementAsset : _data)
		{
			auto & meshElmentRenderData = _renderData->GetMeshElements()[index];

			// Position
			{
				VertexBufferBuilder builder;
				for (auto & vertexDataSlot : meshElementAsset->vertexData)
				{
					auto positionElmentIndex = vertexDataSlot->FindVertexElement(MeshVertexElementSignature::MVET_POSITION, 0);
					if (positionElmentIndex >= 0)
					{
						builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
						builder.SetNumVertices( vertexDataSlot->GetNumVertices() );

						for (int32_t i = 0; i < builder.numVertices; ++i)
						{
							float3 * pPosition = vertexDataSlot->GetElement<float3>(i, MeshVertexElementSignature::MVET_POSITION, 0);
							builder.Add(*pPosition);
						}
						meshElmentRenderData->_depthVertexBuffer.push_back(builder.Finish()->DyCast<VertexBuffer>());
					}
				}
			}
			
			// Texcoord
			{
				VertexBufferBuilder builder;
				for (auto & vertexDataSlot : meshElementAsset->vertexData)
				{
					auto positionElmentIndex = vertexDataSlot->FindVertexElement(MeshVertexElementSignature::MVET_TEXCOORD, 0);
					if (positionElmentIndex >= 0)
					{
						builder.AddElementDesc("TEXCOORD", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
						builder.SetNumVertices(vertexDataSlot->GetNumVertices());

						for (int32_t i = 0; i < builder.numVertices; ++i)
						{
							float3 * pTexcoord = vertexDataSlot->GetElement<float3>(i, MeshVertexElementSignature::MVET_TEXCOORD, 0);
							builder.Add(*pTexcoord);
						}
						meshElmentRenderData->_depthVertexBuffer.push_back(builder.Finish()->DyCast<VertexBuffer>());
					}
				}
			}

			++index;
		}
	}

	Ptr<Actor> Mesh::AddInstanceToScene(
		const Ptr<Scene> & scene,
		const float3 & pos,
		const float3 & scale,
		const Quaternion & orientation)
	{
		auto actor = std::make_shared<Actor>();
		scene->AddActor(actor);

		auto renderCom = std::make_shared<RenderMeshComponent>();
		renderCom->SetMesh(shared_from_this());
		renderCom->SetPos(pos);
		renderCom->SetScale(scale);
		renderCom->SetOrientation(orientation);
		renderCom->UpdateTransform();
		actor->AddComponent(renderCom);

		actor->ActivateAllComponents();

		return actor;
	}

	bool Mesh::IsDirty() const
	{
		return _bDirty || (_renderData && _renderData->IsDirty());
	}

	void Mesh::UpdateFromRenderData()
	{
		if (!_renderData)
			return;

		for (auto & meshElementRender : _renderData->GetMeshElements())
		{
			auto meshElement = meshElementRender->GetElementData();

			// Vertex data
			int32_t slotIndex = 0;
			for (auto & vb : meshElementRender->GetVertexBuffer())
			{
				auto vbSize = vb->GetDataSize();
				meshElement->vertexData[slotIndex]->bufferSize = vbSize;
				meshElement->vertexData[slotIndex]->rawBuffer = MakeBufferedDataShared(vbSize);
				vb->Dump(meshElement->vertexData[slotIndex]->rawBuffer.get());

				++slotIndex;
			}

			// Index
			auto ib = meshElementRender->GetIndexBuffer();
			meshElement->indices.resize(ib->GetDesc().numElements);
			if (ib->GetDesc().elementSize == sizeof(uint32_t))
			{
				ib->Dump(&meshElement->indices[0]);
			}
			else if(ib->GetDesc().elementSize == sizeof(uint16_t))
			{
				auto indexBuf = MakeBufferedDataShared(ib->GetDataSize());
				ib->Dump(indexBuf.get());
				auto pIndex = reinterpret_cast<uint16_t*>( indexBuf.get() );
				for (auto & i : meshElement->indices)
					i = *(pIndex++);
			}
			else
			{
				ToyGE_ASSERT_FAIL("Unexpected index buffer format!");
			}
		}
	}

	Ptr<Actor> AddMeshInstanceToScene(
		const String & meshPath,
		const Ptr<Scene> & scene,
		const float3 & pos,
		const float3 & scale,
		const Quaternion & orientation)
	{
		auto mesh = MeshAsset::FindAndInit<MeshAsset>(meshPath)->GetMesh();
		return mesh->AddInstanceToScene(scene, pos, scale, orientation);
	}


	bool MeshElementRenderData::IsDirty() const
	{
		bool bDirty = _bDirty || (_material && _material->IsDirty()) || _indexBuffer->IsDirty();
		if (!bDirty)
		{
			for (auto & vb : _vertexBuffers)
				if (vb->IsDirty())
					return true;
			return false;
		}
		else
			return true;
	}


	void MeshElementRenderData::BindMacros(std::map<String, String> & outMacros)
	{
		static const std::map<MeshVertexElementSignature, String> mviMacrosMap =
		{
			{ MeshVertexElementSignature::MVET_POSITION,	"MVI_POSITION" },
			{ MeshVertexElementSignature::MVET_TEXCOORD,	"MVI_TEXCOORD" },
			{ MeshVertexElementSignature::MVET_NORMAL,		"MVI_NORMAL" },
			{ MeshVertexElementSignature::MVET_TANGENT,		"MVI_TANGENT" },
			{ MeshVertexElementSignature::MVET_BITANGENT,	"MVI_BITANGENT" },
			{ MeshVertexElementSignature::MVET_COLOR,		"MVI_COLOR" }
		};

		auto meshElement = GetElementData();
		int32_t numTexCoords = 0;
		for (auto vertexSlot : meshElement->vertexData)
		{
			for (auto & vertexElementDesc : vertexSlot->vertexDesc.elementsDesc)
			{
				outMacros[mviMacrosMap.find(vertexElementDesc.signature)->second] = "1";

				if (vertexElementDesc.signature == MeshVertexElementSignature::MVET_TEXCOORD)
					numTexCoords = std::max<int32_t>(numTexCoords, vertexElementDesc.signatureIndex + 1);
			}
		}

		outMacros["NUM_TEXCOORD"] = std::to_string(numTexCoords);
	}

	void MeshElementRenderData::BindShaderParams(const Ptr<Shader> & shader)
	{

	}

	void MeshElementRenderData::Draw()
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetVertexBuffer(_vertexBuffers);
		rc->SetIndexBuffer(_indexBuffer);
		rc->SetPrimitiveTopology(PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		rc->DrawIndexed(0, 0);
	}


	void MeshElementRenderData::BindDepthMacros(bool bWithOpacityMask, std::map<String, String> & outMacros)
	{
		static const std::map<MeshVertexElementSignature, String> mviMacrosMap =
		{
			{ MeshVertexElementSignature::MVET_POSITION,	"MVI_POSITION" },
			{ MeshVertexElementSignature::MVET_TEXCOORD,	"MVI_TEXCOORD" }
		};

		auto meshElement = GetElementData();
		int32_t numTexCoords = 0;
		for (auto & vertexSlot : meshElement->vertexData)
		{
			for (auto & vertexElementDesc : vertexSlot->vertexDesc.elementsDesc)
			{
				if (mviMacrosMap.count(vertexElementDesc.signature) == 0)
					continue;

				if (vertexElementDesc.signature == MeshVertexElementSignature::MVET_TEXCOORD)
				{
					if (bWithOpacityMask)
					{
						numTexCoords = std::max<int32_t>(numTexCoords, vertexElementDesc.signatureIndex + 1);

						outMacros[mviMacrosMap.find(vertexElementDesc.signature)->second] = "1";
					}
				}
				else
				{
					outMacros[mviMacrosMap.find(vertexElementDesc.signature)->second] = "1";
				}
			}
		}

		outMacros["NUM_TEXCOORD"] = std::to_string(numTexCoords);
	}

	void MeshElementRenderData::BindDepthShaderParams(const Ptr<class Shader> & shader, bool bWithOpacityMask)
	{

	}

	void MeshElementRenderData::DrawDepth(bool bWithOpacityMask)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();
		if(bWithOpacityMask)
			rc->SetVertexBuffer(_depthVertexBuffer);
		else
			rc->SetVertexBuffer({ _depthVertexBuffer[0] });
		rc->SetIndexBuffer(_indexBuffer);
		rc->SetPrimitiveTopology(PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		rc->DrawIndexed(0, 0);
	}

	void MeshElementRenderData::InitWeightBuffer()
	{
		VertexBufferBuilder builder;
		builder.AddElementDesc("WEIGHT", 0, RENDER_FORMAT_R32_FLOAT, 0);
		builder.SetNumVertices(_vertexBuffers[0]->GetDesc().numElements);

		auto meshElementAsset = _meshElementData.lock();

		int32_t posVertexSlotIndex = 0;
		for (auto & slot : meshElementAsset->vertexData)
		{
			if (slot->GetElement<float3>(0, MeshVertexElementSignature::MVET_POSITION, 0))
				break;
			else
				++posVertexSlotIndex;
		}

		std::vector<float> weights(builder.numVertices);
		//float all = 0.0f;
		int32_t numTriangles = _indexBuffer->GetDesc().numElements / 3;
		
		for (int32_t triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex)
		{
			auto i0 = meshElementAsset->indices[triangleIndex * 3 + 0];
			auto i1 = meshElementAsset->indices[triangleIndex * 3 + 1];
			auto i2 = meshElementAsset->indices[triangleIndex * 3 + 2];
			auto p0 = *meshElementAsset->vertexData[posVertexSlotIndex]->GetElement<float3>(i0, MeshVertexElementSignature::MVET_POSITION, 0);
			auto p1 = *meshElementAsset->vertexData[posVertexSlotIndex]->GetElement<float3>(i1, MeshVertexElementSignature::MVET_POSITION, 0);
			auto p2 = *meshElementAsset->vertexData[posVertexSlotIndex]->GetElement<float3>(i2, MeshVertexElementSignature::MVET_POSITION, 0);

			float3 v0 = p1 - p0;
			float3 v1 = p2 - p0;
			float3 t = cross(v0, v1);
			float area = length(t) * 0.5f;
			float areaVertex = area / 3.0f;
			weights[i0] += areaVertex;
			weights[i1] += areaVertex;
			weights[i2] += areaVertex;
			//all += area * 0.5f;
		}

		memcpy(builder.dataBuffer.get(), &weights[0], sizeof(float) * weights.size());
		_weightBuffer = builder.Finish();
	}
	
	Ptr<Mesh> CommonMesh::CreatePlane(float width, float height, int32_t uSplits, int32_t vSplits)
	{
		auto mesh = std::make_shared<Mesh>();

		auto meshElement = std::make_shared<MeshElement>();
		meshElement->vertexData.push_back(std::make_shared<MeshVertexSlotData>());

		VertexBufferBuilder builder;
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_POSITION, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("TEXCOORD", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_TEXCOORD, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("NORMAL", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_NORMAL, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("TANGENT", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_TANGENT, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("BITANGENT", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_BITANGENT, 0, i, builder.vertexSize - i });
		}
		meshElement->vertexData[0]->vertexDesc.bytesSize = builder.vertexSize;

		builder.SetNumVertices(uSplits * vSplits * 4);

		float x = -width * 0.5f;
		float z =  height * 0.5f;
		float xStep =  width / static_cast<float>(uSplits);
		float zStep = -height / static_cast<float>(vSplits);
		float u = 0.0f;
		float v = 0.0f;
		float uStep = 1.0f / static_cast<float>(uSplits);
		float vStep = 1.0f / static_cast<float>(vSplits);

		float3 normal = float3(0.0f, 1.0f, 0.0f);
		float3 tangent = float3(1.0f, 0.0f, 0.0f);
		float3 bitangent = float3(0.0f, 0.0f, -1.0f);

		int32_t vertexIndex = 0;
		for (int32_t uSplitIndex = 0; uSplitIndex < uSplits; ++uSplitIndex)
		{
			for (int32_t vSplitIndex = 0; vSplitIndex < vSplits; ++vSplitIndex)
			{
				builder.Add(float3(x, 0.0f, z));
				builder.Add(float3(0.0f, 0.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);
				builder.Add(bitangent);

				builder.Add(float3(x + xStep, 0.0f, z));
				builder.Add(float3(1.0f, 0.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);
				builder.Add(bitangent);

				builder.Add(float3(x + xStep, 0.0f, z + zStep));
				builder.Add(float3(1.0f, 1.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);
				builder.Add(bitangent);

				builder.Add(float3(x, 0.0f, z + zStep));
				builder.Add(float3(0.0f, 1.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);
				builder.Add(bitangent);

				meshElement->indices.push_back(vertexIndex + 0);
				meshElement->indices.push_back(vertexIndex + 1);
				meshElement->indices.push_back(vertexIndex + 2);

				meshElement->indices.push_back(vertexIndex + 0);
				meshElement->indices.push_back(vertexIndex + 2);
				meshElement->indices.push_back(vertexIndex + 3);

				vertexIndex += 4;
				z += zStep;
			}
			z =  height * 0.5f;
			x += xStep;
		}

		meshElement->vertexData[0]->bufferSize = builder.vertexSize * builder.numVertices;
		meshElement->vertexData[0]->rawBuffer = builder.dataBuffer;

		mesh->SetData({ meshElement });
		mesh->Init();
		
		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateSphere(float radius, int32_t numSplits)
	{
		if (numSplits <= 1)
			return Ptr<Mesh>();

		auto mesh = std::make_shared<Mesh>();

		auto meshElement = std::make_shared<MeshElement>();
		meshElement->vertexData.push_back(std::make_shared<MeshVertexSlotData>());

		VertexBufferBuilder builder;
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_POSITION, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("NORMAL", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_NORMAL, 0, i, builder.vertexSize - i });
		}
		meshElement->vertexData[0]->vertexDesc.bytesSize = builder.vertexSize;

		builder.SetNumVertices((numSplits + 1) * (numSplits + 1));

		for (int32_t thetaIndex = 0; thetaIndex <= numSplits; ++thetaIndex)
		{
			float theta = static_cast<float>(thetaIndex) / static_cast<float>(numSplits) * PI;
			for (int32_t phiIndex = 0; phiIndex <= numSplits; ++phiIndex)
			{
				float phi = static_cast<float>(phiIndex) / static_cast<float>(numSplits)* PI2;
				float x = radius * sin(theta) * cos(phi);
				float y = radius * cos(theta);
				float z = radius * sin(theta) * -sin(phi);
				builder.Add(float3(x, y, z));
				builder.Add(normalize(float3(x, y, z)));
			}
		}
		meshElement->vertexData[0]->bufferSize = builder.vertexSize * builder.numVertices;
		meshElement->vertexData[0]->rawBuffer = builder.dataBuffer;

		for (int32_t thetaIndex = 0; thetaIndex < numSplits; ++thetaIndex)
		{
			int32_t offset = thetaIndex * (numSplits + 1);
			for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
			{
				meshElement->indices.push_back(offset + phiIndex);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 1);
				meshElement->indices.push_back(offset + phiIndex + 1);

				meshElement->indices.push_back(offset + phiIndex + 1);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 1);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 2);
			}
		}

		mesh->SetData({ meshElement });
		mesh->Init();

		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateCone(float height, float angle, int32_t numSplits)
	{
		auto mesh = std::make_shared<Mesh>();

		auto meshElement = std::make_shared<MeshElement>();
		meshElement->vertexData.push_back(std::make_shared<MeshVertexSlotData>());

		VertexBufferBuilder builder;
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_POSITION, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("NORMAL", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_NORMAL, 0, i, builder.vertexSize - i });
		}
		meshElement->vertexData[0]->vertexDesc.bytesSize = builder.vertexSize;

		builder.SetNumVertices((numSplits + 1) * (numSplits + 1) + 1);

		//builder.Start();
		for (int32_t hIndex = 0; hIndex <= numSplits; ++hIndex)
		{
			float h = height - static_cast<float>(hIndex) * height / static_cast<float>(numSplits);
			float r = (height - h) * tan(angle * 0.5f);
			for (int32_t phiIndex = 0; phiIndex <= numSplits; ++phiIndex)
			{
				float phi = static_cast<float>(phiIndex) / static_cast<float>(numSplits) * PI2;
				float x = r * sin(phi);
				float y = h;
				float z = r * cos(phi);
				//Pos
				builder.Add(float3(x, y, z));
				//Normal
				builder.Add(float3(sin(phi), sin(angle * 0.5f), cos(phi)));
			}
		}
		builder.Add(float3(0.0f, 0.0f, 0.0f));
		builder.Add(float3(0.0f, -1.0f, 0.0f));

		meshElement->vertexData[0]->bufferSize = builder.vertexSize * builder.numVertices;
		meshElement->vertexData[0]->rawBuffer = builder.dataBuffer;

		for (int32_t hIndex = 0; hIndex < numSplits; ++hIndex)
		{
			int32_t offset = hIndex * (numSplits + 1);
			for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
			{
				meshElement->indices.push_back(offset + phiIndex);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 1);
				meshElement->indices.push_back(offset + phiIndex + 1);

				meshElement->indices.push_back(offset + phiIndex + 1);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 1);
				meshElement->indices.push_back(offset + phiIndex + numSplits + 2);
			}
		}

		int32_t offset = numSplits * (numSplits + 1);
		for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
		{
			meshElement->indices.push_back(offset + phiIndex);
			meshElement->indices.push_back(builder.numVertices - 1);
			meshElement->indices.push_back(offset + phiIndex + 1);
		}

		mesh->SetData({ meshElement });
		mesh->Init();

		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateCube(float width, float height, float depth)
	{
		auto mesh = std::make_shared<Mesh>();

		auto meshElement = std::make_shared<MeshElement>();
		meshElement->vertexData.push_back(std::make_shared<MeshVertexSlotData>());

		VertexBufferBuilder builder;
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_POSITION, 0, i, builder.vertexSize - i });
		}
		{
			int i = builder.vertexSize;
			builder.AddElementDesc("NORMAL", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
			meshElement->vertexData[0]->vertexDesc.elementsDesc.push_back({ MeshVertexElementSignature::MVET_NORMAL, 0, i, builder.vertexSize - i });
		}
		meshElement->vertexData[0]->vertexDesc.bytesSize = builder.vertexSize;

		builder.SetNumVertices(24);

		float3 extent = float3(width, height, depth) * 0.5f;
		float3 pos[8] = 
		{
			float3(-extent.x(), -extent.y(), -extent.z()),
			float3( extent.x(), -extent.y(), -extent.z()),
			float3(-extent.x(),  extent.y(), -extent.z()),
			float3( extent.x(),  extent.y(), -extent.z()),

			float3(-extent.x(), -extent.y(),  extent.z()),
			float3( extent.x(), -extent.y(),  extent.z()),
			float3(-extent.x(),  extent.y(),  extent.z()),
			float3( extent.x(),  extent.y(),  extent.z())
		};

		int32_t idx = 0;

		//Face +X
		builder.Add(pos[1]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[5]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[3]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[7]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);
		idx += 4;

		//Face -X
		builder.Add(pos[4]);
		builder.Add(float3(-1.0f, 0.0f, 0.0f));
		builder.Add(pos[0]);
		builder.Add(float3(-1.0f, 0.0f, 0.0f));
		builder.Add(pos[6]);
		builder.Add(float3(-1.0f, 0.0f, 0.0f));
		builder.Add(pos[2]);
		builder.Add(float3(-1.0f, 0.0f, 0.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);
		idx += 4;

		//Face +Y
		builder.Add(pos[2]);
		builder.Add(float3(0.0f, 1.0f, 0.0f));
		builder.Add(pos[3]);
		builder.Add(float3(0.0f, 1.0f, 0.0f));
		builder.Add(pos[6]);
		builder.Add(float3(0.0f, 1.0f, 0.0f));
		builder.Add(pos[7]);
		builder.Add(float3(0.0f, 1.0f, 0.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);
		idx += 4;

		//Face -Y
		builder.Add(pos[1]);
		builder.Add(float3(0.0f, -1.0f, 0.0f));
		builder.Add(pos[0]);
		builder.Add(float3(0.0f, -1.0f, 0.0f));
		builder.Add(pos[5]);
		builder.Add(float3(0.0f, -1.0f, 0.0f));
		builder.Add(pos[4]);
		builder.Add(float3(0.0f, -1.0f, 0.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);
		idx += 4;

		//Face +Z
		builder.Add(pos[5]);
		builder.Add(float3(0.0f, 0.0f, 1.0f));
		builder.Add(pos[4]);
		builder.Add(float3(0.0f, 0.0f, 1.0f));
		builder.Add(pos[7]);
		builder.Add(float3(0.0f, 0.0f, 1.0f));
		builder.Add(pos[6]);
		builder.Add(float3(0.0f, 0.0f, 1.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);
		idx += 4;

		//Face -Z
		builder.Add(pos[0]);
		builder.Add(float3(0.0f, 0.0f, -1.0f));
		builder.Add(pos[1]);
		builder.Add(float3(0.0f, 0.0f, -1.0f));
		builder.Add(pos[2]);
		builder.Add(float3(0.0f, 0.0f, -1.0f));
		builder.Add(pos[3]);
		builder.Add(float3(0.0f, 0.0f, -1.0f));
		meshElement->indices.push_back(idx + 0);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 1);
		meshElement->indices.push_back(idx + 2);
		meshElement->indices.push_back(idx + 3);
		meshElement->indices.push_back(idx + 1);

		meshElement->vertexData[0]->bufferSize = builder.vertexSize * builder.numVertices;
		meshElement->vertexData[0]->rawBuffer = builder.dataBuffer;

		mesh->SetData({ meshElement });
		mesh->Init();

		return mesh;
	}
}