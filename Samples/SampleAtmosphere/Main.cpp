#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleAtmosphere : public SampleCommon
{
public:
	Ptr<DirectionalLightComponent> _light;
	Ptr<AtmosphereRendering> _atmosRender;
	Ptr<PostProcessVolumetricLight> _ppvl;
	float3 _sunRadiance;
	float3 _sunDir;
	float _ppvlDensity;
	float _ppvlIntensity;
	float _ppvlDecay;

	SampleAtmosphere()
		: _sunRadiance(20.0f),
		_sunDir(0.0f, -0.02f, -1.0f),
		_ppvlDensity(0.8f),
		_ppvlIntensity(0.8f),
		_ppvlDecay(0.9f)
	{
		_sampleName = L"Atmosphere";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_atmosRender = std::make_shared<AtmosphereRendering>();
		Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->SetBackgroundRender(_atmosRender);
		_ppvl = std::make_shared<PostProcessVolumetricLight>();
		_renderView->AddPostProcessRender(_ppvl);
		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto camera =  std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(XMFLOAT3(-0.5f, 1.0f, -3.0f));
		camera->SetFarPlane(1e6);
		//camera->Yaw(XM_PIDIV2);

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto dirLightCom = std::make_shared<DirectionalLightComponent>();
		dirLightCom->SetDirection(XMFLOAT3(_sunDir.x, _sunDir.y, _sunDir.z));
		dirLightCom->SetCastShadow(true);
		auto dirLightObj = std::make_shared<SceneObject>();
		dirLightObj->AddComponent(dirLightCom);
		dirLightObj->ActiveAllComponents();
		scene->AddSceneObject(dirLightObj);
		_light = dirLightCom;

		_atmosRender->SetSunDirection(_light->Direction());
		_atmosRender->SetSunRadiance(XMFLOAT3(_sunRadiance.x, _sunRadiance.y, _sunRadiance.z));
		_light->SetRadiance(_atmosRender->ComputeSunRadianceAt(_light->Direction(), XMFLOAT3(_sunRadiance.x, _sunRadiance.y, _sunRadiance.z), 1.0f));

		_ppvl->SetLight(_light);

		//Add Objs
		std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);

		for (auto obj : objs)
			obj->SetMaterial(mat);

		mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(1.0f);
		mat->SetMetallic(0.0f);
		auto sphere = CommonMesh::CreateSphere(1.0f, 500);
		auto sphereObj = sphere->AddInstanceToScene(
			scene,
			XMFLOAT3(0.0f, -6370000.0f, 0.0f),
			XMFLOAT3(6370000.0f, 6370000.0f, 6370000.0f),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		sphereObj->SetMaterial(mat);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "VolumetricLight");

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "SunDirection", TW_TYPE_DIR3F, &_sunDir, nullptr);

		TwAddVarRW(_twBar, "Density", TW_TYPE_FLOAT, &_ppvlDensity, nullptr);
		TwSetParam(_twBar, "Density", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Density", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Density", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "Density", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Intensity", TW_TYPE_FLOAT, &_ppvlIntensity, nullptr);
		TwSetParam(_twBar, "Intensity", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Intensity", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Intensity", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Decay", TW_TYPE_FLOAT, &_ppvlDecay, nullptr);
		TwSetParam(_twBar, "Decay", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Decay", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Decay", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "Decay", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_light->SetDirection(XMFLOAT3(_sunDir.x, _sunDir.y, _sunDir.z));

		_atmosRender->SetSunDirection(_light->Direction());
		_atmosRender->SetSunRadiance(XMFLOAT3(_sunRadiance.x, _sunRadiance.y, _sunRadiance.z));
		_light->SetRadiance(_atmosRender->ComputeSunRadianceAt(_light->Direction(), XMFLOAT3(_sunRadiance.x, _sunRadiance.y, _sunRadiance.z), 1.0f));

		_ppvl->SetDensity(_ppvlDensity);
		_ppvl->SetIntensity(_ppvlIntensity);
		_ppvl->SetDecay(_ppvlDecay);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleAtmosphere>());

	return 0;
}