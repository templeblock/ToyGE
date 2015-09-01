#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleSSR : public SampleCommon
{
public:
	Ptr<SSR> _ssr;
	bool _enableSSR;
	float _ssrMaxRoughness;
	float _ssrIntensity;

	SampleSSR()
		: _enableSSR(true),
		_ssrMaxRoughness(0.9f),
		_ssrIntensity(5.0f)
	{
		_sampleName = L"SSR";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_ssr = std::make_shared<SSR>();
		_renderView->AddPostProcessRender(_ssr);
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
		auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetRadiance(XMFLOAT3(30.0f, 30.0f, 30.0f));
		pointLightCom->SetCastShadow(true);
		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);

		std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);

		model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);

		for (auto obj : objs)
			obj->SetMaterial(mat);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "SSR");

		TwAddVarRW(_twBar, "EnableSSR", TW_TYPE_BOOLCPP, &_enableSSR, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "SSRMaxRoughness", TW_TYPE_FLOAT, &_ssrMaxRoughness, nullptr);
		TwSetParam(_twBar, "SSRMaxRoughness", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "SSRMaxRoughness", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "SSRMaxRoughness", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.1f;
		TwAddVarRW(_twBar, "SSRIntensity", TW_TYPE_FLOAT, &_ssrIntensity, nullptr);
		TwSetParam(_twBar, "SSRIntensity", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "SSRIntensity", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_ssr->SetEnable(_enableSSR);

		_ssr->SetSSRMaxRoughness(_ssrMaxRoughness);
		_ssr->SetSSRIntensity(_ssrIntensity);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleSSR>());

	return 0;
}