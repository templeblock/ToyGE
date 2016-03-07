#include "SampleCommon.h"

using namespace ToyGE;

class SampleVolumetricLight : public SampleCommon
{
public:
	Ptr<SpotLightComponent> _light;
	//Ptr<VolumetricLight> _vl;
	bool _enableLightVolume;
	float _attenuation;
	float _scattering;
	float _phaseFunctionParam;
	float3 _dir;

	SampleVolumetricLight()
		: _enableLightVolume(true),
		_attenuation(0.1f),
		_scattering(0.5f),
		_phaseFunctionParam(0.5f),
		_dir(0.0f, -1.0f, 0.0f)
	{
		_sampleName = "VolumetricLight";
	}

	void Init() override
	{
		SampleCommon::Init();

		_renderView->sceneRenderingConfig.bTAA = true;

		auto pp = std::make_shared<PostProcessing>();
		/*_vl = std::make_shared<VolumetricLight>();
		pp->AddRender(_vl);*/
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera = _renderView->GetCamera();
		camera->SetPos(float3(-4.0f, 1.5f, 0.0f));
		camera->Yaw(PI_DIV2);

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto spotLight = LightActor::Create<SpotLightComponent>(scene);
		spotLight->GetLight<SpotLightComponent>()->SetPos(float3(0.0f, 3.0f, 0.0f));
		spotLight->GetLight<SpotLightComponent>()->SetDirection(float3(0.0f, -1.0f, 0.0f));
		spotLight->GetLight<SpotLightComponent>()->SetColor(1.0f);
		spotLight->GetLight<SpotLightComponent>()->SetIntensity(30.0f);
		spotLight->GetLight<SpotLightComponent>()->SetDecreaseSpeed(10.0f);
		spotLight->GetLight<SpotLightComponent>()->SetCastShadow(true);
		spotLight->GetLight<SpotLightComponent>()->SetCastLightVolume(true);
		_light = spotLight->GetLight<SpotLightComponent>();

		std::vector<Ptr<RenderComponent>> objs;

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			auto actor = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 1.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		/*auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);

		model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);

		for (auto obj : objs)
			obj->SetMaterial(mat);*/

		auto capture = std::make_shared<ReflectionMapCapture>();
		capture->SetPos(float3(0.0f, 6.0f, 0.0));
		capture->SetRadius(40.0f);
		scene->AddReflectionMapCapture(capture);
		scene->InitReflectionMaps();

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "VolumetricLight");

		TwAddVarRW(_twBar, "EnableVolumetricLight", TW_TYPE_BOOLCPP, &_enableLightVolume, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "Attenuation", TW_TYPE_FLOAT, &_attenuation, nullptr);
		TwSetParam(_twBar, "Attenuation", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Attenuation", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Scaterring", TW_TYPE_FLOAT, &_scattering, nullptr);
		TwSetParam(_twBar, "Scaterring", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Scaterring", "step", TW_PARAM_FLOAT, 1, &step);

		minMax.x() = -1.0f;
		minMax.y() = 1.0f;
		TwAddVarRW(_twBar, "PhaseFunctionParam", TW_TYPE_FLOAT, &_phaseFunctionParam, nullptr);
		TwSetParam(_twBar, "PhaseFunctionParam", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "PhaseFunctionParam", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "PhaseFunctionParam", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Direction", TW_TYPE_DIR3F, &_dir, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_light->SetCastLightVolume(_enableLightVolume);
		_light->SetDirection(_dir);

		if (Global::GetRenderEngine()->GetSceneRenderer()->GetVolumetricLightingRenderer())
		{
			Global::GetRenderEngine()->GetSceneRenderer()->GetVolumetricLightingRenderer()->SetAttenuation(_attenuation);
			Global::GetRenderEngine()->GetSceneRenderer()->GetVolumetricLightingRenderer()->SetScattering(_scattering);
			Global::GetRenderEngine()->GetSceneRenderer()->GetVolumetricLightingRenderer()->SetPhaseFunctionParam(_phaseFunctionParam);
		}
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleVolumetricLight>());

	return 0;
}