#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

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
		SCENE3 = 3UL
	};

	struct SceneData
	{
		Ptr<Texture> tex;
		Ptr<ReflectionMap> reflecMap;
	};

	std::map<SceneType, SceneData> _sceneMap;
	SceneType _curScene;
	Ptr<RenderMeshComponent> _objs;

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
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

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
			"Textures/rnl_cross.dds",
			"Textures/galileo_cross.dds",
			"Textures/uffizi_cross.dds",
			"Textures/stpeters_cross_mmp_ABGR16F.dds"
		};

		for (int32_t i = 0; i < _countof(bkTexs); ++i)
		{
			SceneData sceneData;
			auto texAsset = Asset::Find<TextureAsset>(bkTexs[i]);
			if (!texAsset->IsInit())
				texAsset->Init();
			sceneData.tex = texAsset->GetTexture();

			auto reflectionMap = std::make_shared<ReflectionMap>();
			reflectionMap->SetEnvironmentMap(sceneData.tex);
			reflectionMap->InitPreComputedData();

			sceneData.reflecMap = reflectionMap;

			_sceneMap[static_cast<SceneType>(i)] = sceneData;
		}


		/*auto backgroundRender = std::static_pointer_cast<SkyBox>(Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->GetBackgroundRender());
		backgroundRender->SetTexture(_sceneMap[_curScene].tex);*/

		_mat = std::make_shared<Material>();
		_mat->SetBaseColor(_matBaseColor);
		_mat->SetRoughness(_matRoughness);
		_mat->SetMetallic(_matMetallic);

		auto model = Asset::Find<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
		if (!model->IsInit())
			model->Init();
		_objs = model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

		for (auto & obj : _objs->GetSubRenderComponents())
		{
			obj->SetMaterial(_mat);
			obj->SetReflectionMap(_sceneMap[_curScene].reflecMap);
		}

		auto camera = scene->GetView(0)->GetCamera();
		camera->SetPos(XMFLOAT3(0.0f, 1.0f, 2.0f));
		camera->Yaw(-XM_PI);

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
			{ SCENE3, "Scene3" }
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

		Global::GetScene()->SetAmbientTexture(_sceneMap[_curScene].tex);

		for (auto & obj : _objs->GetSubRenderComponents())
		{
			obj->SetMaterial(_mat);
			obj->SetReflectionMap(_sceneMap[_curScene].reflecMap);
		}
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleIBL>());

	return 0;
}