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
		_ppvlDecay(0.6f)
	{
		_sampleName = "Atmosphere";
	}

	void Init() override
	{
		SampleCommon::Init();

		Global::GetRenderEngine()->GetSceneRenderer()->bRenderingAtmosphere = true;

		auto pp = std::make_shared<PostProcessing>();

		_ppvl = std::make_shared<PostProcessVolumetricLight>();
		pp->AddRender(_ppvl);
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera =  std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(XMFLOAT3(-0.5f, 0.5f, -3.0f));
		camera->SetFar(1e4);

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto dirLightCom = std::make_shared<DirectionalLightComponent>();
		dirLightCom->SetCastShadow(true);
		auto dirLightObj = std::make_shared<SceneObject>();
		dirLightObj->AddComponent(dirLightCom);
		dirLightObj->ActiveAllComponents();
		scene->AddSceneObject(dirLightObj);
		_light = dirLightCom;

		Global::GetRenderEngine()->GetSceneRenderer()->SetSunLight(dirLightCom);
		_ppvl->SetLight(_light);

		//Add Objs
		{
			auto model = Asset::Find<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			if (!model->IsInit())
				model->Init();
			auto objs = model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : objs->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		{
			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(1.0f);
			mat->SetMetallic(0.0f);
			auto sphere = CommonMesh::CreateSphere(1.0f, 500);
			auto sphereObj = sphere->AddInstanceToScene(
				scene,
				XMFLOAT3(0.0f, -6370000.0f, 0.0f),
				XMFLOAT3(6370000.0f, 6370000.0f, 6370000.0f),
				XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			for (auto & obj : sphereObj->GetSubRenderComponents())
			{
				obj->SetMaterial(mat);
				obj->SetCastShadows(false);
			}
		}

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "VolumetricLight");

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "SunDirection", TW_TYPE_DIR3F, &_sunDir, nullptr);

		TwAddVarRW(_twBar, "Density", TW_TYPE_FLOAT, &_ppvlDensity, nullptr);
		TwSetParam(_twBar, "Density", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Density", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Density", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Density", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Intensity", TW_TYPE_FLOAT, &_ppvlIntensity, nullptr);
		TwSetParam(_twBar, "Intensity", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Intensity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Intensity", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Decay", TW_TYPE_FLOAT, &_ppvlDecay, nullptr);
		TwSetParam(_twBar, "Decay", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Decay", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Decay", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Decay", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		Global::GetRenderEngine()->GetSceneRenderer()->SetSunRadiance(XMFLOAT3(_sunRadiance.x(), _sunRadiance.y(), _sunRadiance.z()));
		Global::GetRenderEngine()->GetSceneRenderer()->SetSunDirection(XMFLOAT3(_sunDir.x(), _sunDir.y(), _sunDir.z()));

		Global::GetRenderEngine()->GetSceneRenderer()->UpdateSunLight();

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