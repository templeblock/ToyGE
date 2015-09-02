#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleVolumetricLight : public SampleCommon
{
public:
	Ptr<SpotLightComponent> _light;
	Ptr<VolumetricLight> _vl;
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
		_sampleName = L"VolumetricLight";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_vl = std::make_shared<VolumetricLight>();
		_renderView->AddPostProcessRender(_vl);
		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto camera = _renderView->GetCamera();
		camera->SetPos(XMFLOAT3(-6.0f, 0.5f, 0.0f));
		camera->Yaw(XM_PIDIV2);

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto spotLightCom = std::make_shared<SpotLightComponent>();
		spotLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		spotLightCom->SetDirection(XMFLOAT3(0.0f, -1.0f, 0.0f));
		spotLightCom->SetRadiance(XMFLOAT3(10.0f, 10.0f, 10.0f));
		spotLightCom->SetDecreaseSpeed(10.0f);
		spotLightCom->SetCastShadow(true);
		spotLightCom->SetCastLightVolume(true);
		auto spotLightObj = std::make_shared<SceneObject>();
		spotLightObj->AddComponent(spotLightCom);
		spotLightObj->ActiveAllComponents();
		scene->AddSceneObject(spotLightObj);
		_light = spotLightCom;

		std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);

		model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);

		for (auto obj : objs)
			obj->SetMaterial(mat);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "VolumetricLight");

		TwAddVarRW(_twBar, "EnableVolumetricLight", TW_TYPE_BOOLCPP, &_enableLightVolume, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "Attenuation", TW_TYPE_FLOAT, &_attenuation, nullptr);
		TwSetParam(_twBar, "Attenuation", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Attenuation", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Scaterring", TW_TYPE_FLOAT, &_scattering, nullptr);
		TwSetParam(_twBar, "Scaterring", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Scaterring", "step", TW_PARAM_FLOAT, 1, &step);

		minMax.x = -1.0f;
		minMax.y = 1.0f;
		TwAddVarRW(_twBar, "PhaseFunctionParam", TW_TYPE_FLOAT, &_phaseFunctionParam, nullptr);
		TwSetParam(_twBar, "PhaseFunctionParam", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "PhaseFunctionParam", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "PhaseFunctionParam", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Direction", TW_TYPE_DIR3F, &_dir, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_light->SetCastLightVolume(_enableLightVolume);
		_light->SetDirection(XMFLOAT3(_dir.x, _dir.y, _dir.z));

		_vl->SetAttenuation(_attenuation);
		_vl->SetScattering(_scattering);
		_vl->SetPhaseFunctionParam(_phaseFunctionParam);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleVolumetricLight>());

	return 0;
}