#include "SampleCommon.h"

using namespace ToyGE;

class SampleIBL : public SampleCommon
{
public:
	Ptr<Material> _mat;
	float3 _matBaseColor;
	float _matRoughness;
	float _matMetallic;

	enum SceneType
	{
		SCENE0 = 0UL,
		SCENE1 = 1UL,
		SCENE2 = 2UL,
		SCENE3 = 3UL,
		SCENE4 = 4UL,
		SCENE5 = 5UL
	};

	struct SceneData
	{
		Ptr<Texture> tex;
		//Ptr<ReflectionMap> reflecMap;
	};

	std::map<SceneType, SceneData> _sceneMap;
	SceneType _curScene;
	Ptr<Actor> _actor;
	//Ptr<ReflectionMapCapture> _capture;

	SampleIBL()
		: _matBaseColor(1.0f),
		_matRoughness(0.0f),
		_matMetallic(1.0f),
		_curScene(SCENE0)
	{
		_sampleName = "IBL";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		_renderView->sceneRenderingConfig.bSSR = false;

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		//auto pointLightCom = std::make_shared<PointLightComponent>();
		//pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		//pointLightCom->SetRadiance(XMFLOAT3(30.0f, 30.0f, 30.0f));
		//pointLightCom->SetCastShadow(true);
		//auto pointLightObj = std::make_shared<SceneObject>();
		//pointLightObj->AddComponent(pointLightCom);
		//pointLightObj->ActiveAllComponents();
		//scene->AddSceneObject(pointLightObj);

		String bkTexs[] = 
		{
			"Textures/CubeMaps/pisa.dds",
			"Textures/CubeMaps/uffizi_cross.dds",
			"Textures/CubeMaps/doge2.dds",
			"Textures/CubeMaps/ennis.dds",
			"Textures/CubeMaps/glacier.dds",
			"Textures/CubeMaps/grace-new.dds"
		};

		for (int32_t i = 0; i < _countof(bkTexs); ++i)
		{
			SceneData sceneData;
			auto texAsset = Asset::FindAndInit<TextureAsset>(bkTexs[i]);
			sceneData.tex = texAsset->GetTexture();

			/*auto reflectionMap = std::make_shared<ReflectionMap>();
			reflectionMap->SetEnvironmentMap(sceneData.tex);
			reflectionMap->InitPreComputedData();

			sceneData.reflecMap = reflectionMap;*/

			_sceneMap[static_cast<SceneType>(i)] = sceneData;
		}


		/*auto backgroundRender = std::static_pointer_cast<SkyBox>(Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->GetBackgroundRender());
		backgroundRender->SetTexture(_sceneMap[_curScene].tex);*/

		_mat = std::make_shared<Material>();
		_mat->SetBaseColor(_matBaseColor);
		_mat->SetRoughness(_matRoughness);
		_mat->SetMetallic(_matMetallic);

		/*auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
		_actor = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));*/
		auto mesh = CommonMesh::CreateSphere(1.0f, 50);
		_actor = mesh->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.5f, 0.5f, 0.5f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

		for (auto & obj : _actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
		{
			obj->SetMaterial(_mat);
			//obj->SetReflectionMap(_sceneMap[_curScene].reflecMap);
		}

		auto camera = scene->GetView(0)->GetCamera();
		camera->SetPos(float3(0.0f, 1.0f, 2.0f));
		camera->Yaw(-PI);

		/*_capture = std::make_shared<ReflectionMapCapture>();
		_capture->SetPos(0.0f);
		_capture->SetRadius(20.0f);
		scene->AddReflectionMapCapture(_capture);
		scene->InitReflectionMaps();*/

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "IBL");

		TwAddVarRW(_twBar, "BaseColor", TW_TYPE_COLOR3F, &_matBaseColor, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;
		TwAddVarRW(_twBar, "Roughness", TW_TYPE_FLOAT, &_matRoughness, nullptr);
		TwSetParam(_twBar, "Roughness", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Roughness", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Roughness", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Metallic", TW_TYPE_FLOAT, &_matMetallic, nullptr);
		TwSetParam(_twBar, "Metallic", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Metallic", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Metallic", "step", TW_PARAM_FLOAT, 1, &step);

		TwEnumVal sceneEnumVal[] =
		{
			{ SCENE0, "Scene0" },
			{ SCENE1, "Scene1" },
			{ SCENE2, "Scene2" },
			{ SCENE3, "Scene3" },
			{ SCENE4, "Scene4" },
			{ SCENE5, "Scene5" }
		};
		auto sceneEnumType = TwDefineEnum("SceneType", sceneEnumVal, _countof(sceneEnumVal));
		TwAddVarRW(_twBar, "Scene", sceneEnumType, &_curScene, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_mat->SetBaseColor(_matBaseColor);
		_mat->SetRoughness(_matRoughness);
		_mat->SetMetallic(_matMetallic);

		if (Global::GetScene()->GetAmbientTexture() != _sceneMap[_curScene].tex)
		{
			Global::GetScene()->SetAmbientTexture(_sceneMap[_curScene].tex);
			//Global::GetScene()->InitReflectionMaps();
		}

		for (auto & obj : _actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
		{
			obj->SetMaterial(_mat);
			//obj->SetReflectionMap(_sceneMap[_curScene].reflecMap);
		}

		//ReflectionMap::InitLUT();
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleIBL>());

	return 0;
}