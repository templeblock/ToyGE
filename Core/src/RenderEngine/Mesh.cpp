#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\SceneObject.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
	String StandardVertexElementName::Position()
	{
		return "POSITION";
	}

	String StandardVertexElementName::TextureCoord()
	{
		return "TEXCOORD";
	}

	String StandardVertexElementName::Normal()
	{
		return "NORMAL";
	}

	String StandardVertexElementName::Tangent()
	{
		return "TANGENT";
	}


	VertexDataBuildHelp::VertexDataBuildHelp()
		: _pBufferData(nullptr),
		_bytesOffset(0)
	{

	}

	void VertexDataBuildHelp::AddElementDesc(const String & name, int32_t index, RenderFormat format, int32_t instanceRate)
	{
		VertexElementDesc desc;
		desc.name = name;
		desc.index = index;
		desc.format = format;
		desc.instanceDataRate = instanceRate;
		desc.bytesOffset = _bytesOffset;
		desc.bytesSize = GetRenderFormatNumBits(format) / 8;
		vertexDataDesc.elementsDesc.push_back(desc);
		_bytesOffset += desc.bytesSize;
	}

	void VertexDataBuildHelp::SetNumVertices(int32_t numVertices)
	{
		vertexDataDesc.numVertices = numVertices;
	}

	void VertexDataBuildHelp::Start()
	{
		vertexDataDesc.vertexByteSize = _bytesOffset;
		_buffer = MakeBufferedDataShared(static_cast<size_t>(vertexDataDesc.numVertices * vertexDataDesc.vertexByteSize));
		_pBufferData = _buffer.get();
	}

	void VertexDataBuildHelp::Finish()
	{
		vertexDataDesc.pData = _buffer;
		_pBufferData = nullptr;
		_bytesOffset = 0;
	}


	void Mesh::SaveBin(const Ptr<Writer> & writer, const std::map<Ptr<Material>, int32_t> & materials)
	{
		writer->WriteString(_name);
		//writer->Write<uint32_t>(_dataFlag);
		writer->Write<int32_t>(static_cast<int32_t>(_vertexDatas.size()));
		for (auto & slot : _vertexDatas)
		{
			writer->Write<int32_t>(slot.numVertices);
			writer->Write<int32_t>(slot.vertexByteSize);
			writer->Write<int32_t>(static_cast<int32_t>(slot.elementsDesc.size()));
			for (auto & elemDesc : slot.elementsDesc)
			{
				writer->WriteString(elemDesc.name);
				writer->Write<int32_t>(elemDesc.index);
				writer->Write<uint32_t>(elemDesc.format);
				writer->Write<int32_t>(elemDesc.instanceDataRate);
				writer->Write<int32_t>(elemDesc.bytesOffset);
				writer->Write<int32_t>(elemDesc.bytesSize);
			}
			if (slot.numVertices > 0)
				writer->WriteBytes(slot.pData.get(), static_cast<size_t>(slot.numVertices * slot.vertexByteSize));
		}

		writer->Write<int32_t>(static_cast<int32_t>(_indices.size()));
		if (_indices.size() > 0)
			writer->WriteBytes(&_indices[0], sizeof(uint32_t) * _indices.size());

		//writer->Write<int32_t>(static_cast<int32_t>(_uvChannelsNumComs.size()));
		//if (_uvChannelsNumComs.size() > 0)
		//	writer->WriteBytes(&_uvChannelsNumComs[0], sizeof(uint8_t) * _uvChannelsNumComs.size());
	}

	Ptr<Mesh> Mesh::LoadBin(const Ptr<Reader> & reader, const std::vector<Ptr<Material>> & materials)
	{
		auto mesh = std::make_shared<Mesh>();

		reader->ReadString(mesh->_name);

		//uint32_t dataFlag = reader->Read<uint32_t>();
		//mesh->_dataFlag = dataFlag;

		int32_t numSlots = reader->Read<int32_t>();
		mesh->_vertexDatas.resize(numSlots);
		for (int32_t slotIndex = 0; slotIndex < numSlots; ++slotIndex)
		{
			auto & vertexDataDesc = mesh->_vertexDatas[slotIndex];
			vertexDataDesc.numVertices = reader->Read<int32_t>();
			vertexDataDesc.vertexByteSize = reader->Read<int32_t>();
			auto numElemDescs = reader->Read<int32_t>();
			vertexDataDesc.elementsDesc.resize(numElemDescs);
			for (int32_t elemIndex = 0; elemIndex < numElemDescs; ++elemIndex)
			{
				auto & elemDesc = vertexDataDesc.elementsDesc[elemIndex];
				reader->ReadString(elemDesc.name);
				elemDesc.index = reader->Read<int32_t>();
				elemDesc.format = static_cast<RenderFormat>(reader->Read<uint32_t>());
				elemDesc.instanceDataRate = reader->Read<int32_t>();
				elemDesc.bytesOffset = reader->Read<int32_t>();
				elemDesc.bytesSize = reader->Read<int32_t>();
			}
			auto dataSize = static_cast<size_t>(vertexDataDesc.numVertices * vertexDataDesc.vertexByteSize);
			if (dataSize > 0)
			{
				auto dataBuffer = MakeBufferedDataShared(dataSize);
				reader->ReadBytes(dataBuffer.get(), dataSize);
				vertexDataDesc.pData = dataBuffer;
			}
		}

		int32_t numIndices = reader->Read<int32_t>();
		mesh->_indices.resize(numIndices);
		if (numIndices > 0)
			reader->ReadBytes(&mesh->_indices[0], sizeof(uint32_t) * numIndices);

		//int32_t numUVChannels = reader->Read<int32_t>();
		//mesh->_uvChannelsNumComs.resize(numUVChannels);
		//if (numUVChannels > 0)
		//	reader->ReadBytes(&mesh->_uvChannelsNumComs[0], sizeof(uint8_t) * numUVChannels);

		/*int matIndex = reader->Read<int32_t>();
		if (matIndex >= 0)
		{
			mesh->_material = materials[matIndex];
		}
		else
		{
			mesh->SetMaterial(std::make_shared<Material>());
		}*/

		return mesh;
	}

	const Ptr<RenderMesh> & Mesh::InitRenderData()
	{
		_renderData = std::make_shared<RenderMesh>(shared_from_this());
		return _renderData;
	}

	Ptr<RenderComponent> Mesh::AddInstanceToScene(
		const Ptr<Scene> & scene,
		const XMFLOAT3 & pos,
		const XMFLOAT3 & scale,
		const XMFLOAT4 & orientation)
	{
		auto sceneObj = std::make_shared<SceneObject>();
		scene->AddSceneObject(sceneObj);

		auto renderCom = std::make_shared<RenderComponent>();
		renderCom->SetMesh(shared_from_this());
		renderCom->SetPos(pos);
		renderCom->SetScale(scale);
		renderCom->SetOrientation(orientation);
		renderCom->UpdateTransform();
		sceneObj->AddComponent(renderCom);

		sceneObj->ActiveAllComponents();

		return renderCom;
	}

	Ptr<Mesh> Mesh::Copy() const
	{
		auto mesh = std::make_shared<Mesh>();
		mesh->_vertexDatas = _vertexDatas;
		mesh->_indices = _indices;
		return mesh;
	}

	RenderMesh::RenderMesh(const Ptr<Mesh> & mesh)
	{
		auto factory = Global::GetRenderEngine()->GetRenderFactory();

		//Create Vertex Buffers
		std::vector<Ptr<RenderBuffer>> verticesBuffers;
		RenderBufferDesc bufferDesc;
		bufferDesc.bindFlag = BUFFER_BIND_VERTEX | BUFFER_BIND_IMMUTABLE;
		bufferDesc.cpuAccess = 0;
		bufferDesc.structedByteStride = 0;
		for (int32_t slotIndex = 0; slotIndex < mesh->NumVertexSlots(); ++slotIndex)
		{
			auto & vertexData = mesh->GetVertexData(slotIndex);
			bufferDesc.elementSize = vertexData.vertexByteSize;
			bufferDesc.numElements = vertexData.numVertices;

			verticesBuffers.push_back(factory->CreateBuffer(bufferDesc, vertexData.pData.get()));
			verticesBuffers.back()->SetVertexBufferType(VERTEX_BUFFER_GEOMETRY);
			verticesBuffers.back()->SetVertexElementsDesc(vertexData.elementsDesc);
		}

		//Create Index Buffer
		bufferDesc.bindFlag = BUFFER_BIND_INDEX | BUFFER_BIND_IMMUTABLE;
		bufferDesc.elementSize = sizeof(uint32_t);
		bufferDesc.numElements = mesh->NumIndices();
		auto indicesBuffer = factory->CreateBuffer(bufferDesc, &mesh->_indices[0]);

		_input = factory->CreateRenderInput();
		_input->SetVerticesBuffers(verticesBuffers);
		_input->SetIndicesBuffers(indicesBuffer);
		_input->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}



	Ptr<Mesh> CommonMesh::CreatePlane(float width, float height, int32_t uSplits, int32_t vSplits)
	{
		auto mesh = std::make_shared<Mesh>();
		VertexDataBuildHelp builder;
		builder.AddElementDesc(StandardVertexElementName::Position(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::TextureCoord(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::Normal(),   0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::Tangent(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);

		builder.SetNumVertices(uSplits * vSplits * 4);

		//mesh->AddDataFlag(MESH_DATA_POS);
		//mesh->AddDataFlag(MESH_DATA_NORMAL);
		//mesh->AddDataFlag(MESH_DATA_TANGENT);
		//mesh->AddDataFlag(MESH_DATA_UV);

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

		builder.Start();
		int32_t vertexIndex = 0;
		for (int32_t uSplitIndex = 0; uSplitIndex < uSplits; ++uSplitIndex)
		{
			for (int32_t vSplitIndex = 0; vSplitIndex < vSplits; ++vSplitIndex)
			{
				builder.Add(float3(x, 0.0f, z));
				builder.Add(float3(0.0f, 0.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);

				builder.Add(float3(x + xStep, 0.0f, z));
				builder.Add(float3(1.0f, 0.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);

				builder.Add(float3(x + xStep, 0.0f, z + zStep));
				builder.Add(float3(1.0f, 1.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);

				builder.Add(float3(x, 0.0f, z + zStep));
				builder.Add(float3(0.0f, 1.0f, 0.0f));
				builder.Add(normal);
				builder.Add(tangent);
				/*vertex.pos = float3(x, 0.0f, z);
				vertex.texCoord[0] = float3(0.0f, 0.0f, 0.0f);
				mesh->AddVertex(vertex);
				vertex.pos = float3(x + xStep, 0.0f, z);
				vertex.texCoord[0] = float3(1.0f, 0.0f, 0.0f);
				mesh->AddVertex(vertex);
				vertex.pos = float3(x + xStep, 0.0f, z - zStep);
				vertex.texCoord[0] = float3(1.0f, 1.0f, 0.0f);
				mesh->AddVertex(vertex);
				vertex.pos = float3(x, 0.0f, z - zStep);
				vertex.texCoord[0] = float3(0.0f, 1.0f, 0.0f);
				mesh->AddVertex(vertex);*/

				mesh->AddIndex(vertexIndex + 0);
				mesh->AddIndex(vertexIndex + 1);
				mesh->AddIndex(vertexIndex + 2);

				mesh->AddIndex(vertexIndex + 0);
				mesh->AddIndex(vertexIndex + 2);
				mesh->AddIndex(vertexIndex + 3);

				vertexIndex += 4;
				z += zStep;
			}
			z =  height * 0.5f;
			x += xStep;
		}

		builder.Finish();
		mesh->AddVertexData(builder.vertexDataDesc);

		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateSphere(float radius, int32_t numSplits)
	{
		if (numSplits <= 1)
			return Ptr<Mesh>();

		auto mesh = std::make_shared<Mesh>();
		VertexDataBuildHelp builder;
		builder.AddElementDesc(StandardVertexElementName::Position(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::Normal(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		//mesh->AddDataFlag(MESH_DATA_POS);
		//mesh->AddDataFlag(MESH_DATA_NORMAL);

		builder.SetNumVertices((numSplits + 1) * (numSplits + 1));

		builder.Start();
		//MeshVertex vertex;
		for (int32_t thetaIndex = 0; thetaIndex <= numSplits; ++thetaIndex)
		{
			float theta = static_cast<float>(thetaIndex) / static_cast<float>(numSplits) * XM_PI;
			for (int32_t phiIndex = 0; phiIndex <= numSplits; ++phiIndex)
			{
				float phi = static_cast<float>(phiIndex) / static_cast<float>(numSplits)* XM_2PI;
				float x = radius * sin(theta) * sin(phi);
				float y = radius * cos(theta);
				float z = radius * sin(theta) * cos(phi);
				builder.Add(normalize(float3(x, y, z)));
				builder.Add(normalize(float3(x, y, z)));
				/*vertex.pos = float3(x, y, z);
				vertex.normal = float3(x, y, z);
				mesh->AddVertex(vertex);*/
			}
		}
		builder.Finish();
		mesh->AddVertexData(builder.vertexDataDesc);

		for (int32_t thetaIndex = 0; thetaIndex < numSplits; ++thetaIndex)
		{
			int32_t offset = thetaIndex * (numSplits + 1);
			for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
			{
				mesh->AddIndex(offset + phiIndex);
				mesh->AddIndex(offset + phiIndex + numSplits + 1);
				mesh->AddIndex(offset + phiIndex + 1);

				mesh->AddIndex(offset + phiIndex + 1);
				mesh->AddIndex(offset + phiIndex + numSplits + 1);
				mesh->AddIndex(offset + phiIndex + numSplits + 2);
			}
		}

		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateCone(float height, float angle, int32_t numSplits)
	{
		auto mesh = std::make_shared<Mesh>();
		VertexDataBuildHelp builder;
		builder.AddElementDesc(StandardVertexElementName::Position(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::Normal(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);

		builder.SetNumVertices((numSplits + 1) * (numSplits + 1) + 1);

		builder.Start();
		for (int32_t hIndex = 0; hIndex <= numSplits; ++hIndex)
		{
			float h = height - static_cast<float>(hIndex) * height / static_cast<float>(numSplits);
			float r = (height - h) * tan(angle * 0.5f);
			for (int32_t phiIndex = 0; phiIndex <= numSplits; ++phiIndex)
			{
				float phi = static_cast<float>(phiIndex) / static_cast<float>(numSplits) * XM_2PI;
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

		builder.Finish();
		mesh->AddVertexData(builder.vertexDataDesc);

		for (int32_t hIndex = 0; hIndex < numSplits; ++hIndex)
		{
			int32_t offset = hIndex * (numSplits + 1);
			for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
			{
				mesh->AddIndex(offset + phiIndex);
				mesh->AddIndex(offset + phiIndex + numSplits + 1);
				mesh->AddIndex(offset + phiIndex + 1);

				mesh->AddIndex(offset + phiIndex + 1);
				mesh->AddIndex(offset + phiIndex + numSplits + 1);
				mesh->AddIndex(offset + phiIndex + numSplits + 2);
			}
		}

		int32_t offset = numSplits * (numSplits + 1);
		for (int32_t phiIndex = 0; phiIndex < numSplits; ++phiIndex)
		{
			mesh->AddIndex(offset + phiIndex);
			mesh->AddIndex(builder.vertexDataDesc.numVertices - 1);
			mesh->AddIndex(offset + phiIndex + 1);
		}

		return mesh;
	}

	Ptr<Mesh> CommonMesh::CreateCube(float width, float height, float depth)
	{
		auto mesh = std::make_shared<Mesh>();
		VertexDataBuildHelp builder;
		builder.AddElementDesc(StandardVertexElementName::Position(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc(StandardVertexElementName::Normal(), 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);

		builder.SetNumVertices(24);

		float3 extent = float3(width, height, depth) * 0.5f;
		float3 pos[8] = 
		{
			float3(-extent.x, -extent.y, -extent.z),
			float3( extent.x, -extent.y, -extent.z),
			float3(-extent.x,  extent.y, -extent.z),
			float3( extent.x,  extent.y, -extent.z),

			float3(-extent.x, -extent.y,  extent.z),
			float3( extent.x, -extent.y,  extent.z),
			float3(-extent.x,  extent.y,  extent.z),
			float3( extent.x,  extent.y,  extent.z)
		};

		int32_t idx = 0;

		builder.Start();

		//Face +X
		builder.Add(pos[1]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[5]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[3]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		builder.Add(pos[7]);
		builder.Add(float3(1.0f, 0.0f, 0.0f));
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);
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
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);
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
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);
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
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);
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
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);
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
		mesh->AddIndex(idx + 0);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 1);
		mesh->AddIndex(idx + 2);
		mesh->AddIndex(idx + 3);
		mesh->AddIndex(idx + 1);

		builder.Finish();
		mesh->AddVertexData(builder.vertexDataDesc);

		return mesh;
	}
}