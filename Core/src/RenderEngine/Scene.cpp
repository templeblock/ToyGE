#include "ToyGE\RenderEngine\Scene.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngineInclude.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\ReflectionMapCapture.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Kernel\Assertion.h"
#include "ToyGE\RenderEngine\Actor.h"

namespace ToyGE
{
	int32_t Scene::actorID = 0;

	Scene::Scene()
		: _ambientColor(0.0f)
	{

	}

	Scene::~Scene()
	{

	}

	void Scene::Tick(float elapsedTime)
	{
		for (auto & actor : _actorsMap)
		{
			actor.second->Tick(elapsedTime);
		}
	}

	int32_t Scene::AddActor(const Ptr<Actor> & actor)
	{
		_actorsMap[actorID] = actor;
		actor->SetScene(shared_from_this());
		return actorID++;
	}

	Ptr<Actor> Scene::GetActor(int32_t objID) const
	{
		auto objFind = _actorsMap.find(objID);
		if (objFind != _actorsMap.end())
			return objFind->second;
		else
			return nullptr;
	}

	void Scene::RemoveActor(const Ptr<Actor> & obj)
	{
		_actorsMap.erase(std::find_if(_actorsMap.begin(), _actorsMap.end(),
			[&](const std::pair<int32_t, Ptr<Actor>> & e) -> bool
		{
			return e.second == obj;
		}));
	}

	void Scene::RemoveActor(int32_t objID)
	{
		_actorsMap.erase(objID);
	}

	void Scene::AddView(const Ptr<RenderView> & view)
	{
		_views.push_back(view);
		view->SetScene(shared_from_this());
	}

	void Scene::SetAmbientMapTexture(const Ptr<class Texture> & ambientMapTex, AmbientMapType type)
	{
		if (!_ambientMap || type != _ambientMap->GetType())
		{
			_ambientMap = AmbientMap::Create(type);
		}
		_ambientMap->SetTexture(ambientMapTex);
	}

	void Scene::UpdateAmbientReflectionMap(const Ptr<Texture> & reflectionTex)
	{
		if (reflectionTex)
		{
			if (!_ambientReflectionMap)
				_ambientReflectionMap = std::make_shared<ReflectionMap>();
			_ambientReflectionMap->SetEnvironmentMap(reflectionTex);
			_ambientReflectionMap->InitPreComputedData();
		}
		else
		{
			_ambientReflectionMap = nullptr;
		}
	}

	void Scene::InitReflectionMaps()
	{
		if (_reflectionMapCaptures.size() == 0)
		{
			_reflectionMaps = nullptr;
			_capturesPosRadiusBuffer = nullptr;
			return;
		}

		std::sort(_reflectionMapCaptures.begin(), _reflectionMapCaptures.end(),
			[](const Ptr<ReflectionMapCapture> & capture0, const Ptr<ReflectionMapCapture> & capture1)
		{
			return capture0->GetRadius() < capture1->GetRadius();
		});

		TextureDesc texDesc;
		texDesc.width = texDesc.height = 128;
		texDesc.depth = 1;
		texDesc.arraySize = (int32_t)_reflectionMapCaptures.size();
		texDesc.bCube = true;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.mipLevels = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		_reflectionMaps = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_reflectionMaps->SetDesc(texDesc);
		_reflectionMaps->Init();

		std::vector<float4> bufData;

		int captureIndex = 0;
		for (auto & capture : _reflectionMapCaptures)
		{
			auto reflectionMap = capture->CaptureScene(shared_from_this());
			auto filteredEnvMapRef = reflectionMap->GetPrefiltedEnviromentMap();
			auto filteredEnvMap = filteredEnvMapRef->Get()->Cast<Texture>();

			for (int i = 0; i < 6; ++i)
			{
				for (int mipLevel = 0; mipLevel < filteredEnvMap->GetDesc().mipLevels; ++mipLevel)
				{
					ToyGE_ASSERT(filteredEnvMap->CopyTo(_reflectionMaps, mipLevel, captureIndex * 6 + i, 0, 0, 0, mipLevel, i));
				}
			}

			bufData.push_back(float4(capture->GetPos().x(), capture->GetPos().y(), capture->GetPos().z(), capture->GetRadius()));

			++captureIndex;
		}

		RenderBufferDesc bufDesc;
		bufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE;
		bufDesc.bStructured = false;
		bufDesc.cpuAccess = 0;
		bufDesc.elementSize = (int32_t)sizeof(float4);
		bufDesc.numElements = (int32_t)_reflectionMapCaptures.size();
		_capturesPosRadiusBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		_capturesPosRadiusBuffer->SetDesc(bufDesc);
		_capturesPosRadiusBuffer->Init(&bufData[0]);
	}
}