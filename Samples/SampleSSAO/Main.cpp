#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleSSAO : public SampleCommon
{
public:
	Ptr<SSAO> _ssao;
	bool _enableSSAO;
	float _aoRadius;
	float _aoPower;
	float _aoIntensity;
	Ptr<SharedParamRender> _paramRender;
	bool _aoOnly;

	SampleSSAO()
		: _enableSSAO(true),
		_aoRadius(0.05f),
		_aoPower(10.0f),
		_aoIntensity(0.5f),
		_aoOnly(false)
	{
		_sampleName = L"SSAO";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_ssao = std::make_shared<SSAO>();
		_renderView->AddPostProcessRender(_ssao);


		_renderView->AddPostProcessRender(std::make_shared<HDR>());

		_paramRender = std::make_shared<SharedParamRender>();
		_paramRender->SetRenderParam("AOTex");
		_paramRender->SetRenderParamColorWrite(COLOR_WRITE_R);
		_renderView->AddPostProcessRender(_paramRender);

		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto camera = _renderView->GetCamera();
		camera->SetPos(XMFLOAT3(10.0f, 2.0f, 0.0f));
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

		//Add Objs
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
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "SSAO");

		TwAddVarRW(_twBar, "EnableSSAO", TW_TYPE_BOOLCPP, &_enableSSAO, nullptr);

		TwAddVarRW(_twBar, "ShowAO", TW_TYPE_BOOLCPP, &_aoOnly, nullptr);

		float2 minMax = float2(0.0f, 0.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "AORadius", TW_TYPE_FLOAT, &_aoRadius, nullptr);
		TwSetParam(_twBar, "AORadius", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "AORadius", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.1f;
		TwAddVarRW(_twBar, "AOPower", TW_TYPE_FLOAT, &_aoPower, nullptr);
		TwSetParam(_twBar, "AOPower", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "AOPower", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.01f;
		TwAddVarRW(_twBar, "AOIntensity", TW_TYPE_FLOAT, &_aoIntensity, nullptr);
		TwSetParam(_twBar, "AOIntensity", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "AOIntensity", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_ssao->SetEnable(_enableSSAO);

		_ssao->SetAORadius(_aoRadius);
		_ssao->SetAOPower(_aoPower);
		_ssao->SetAOIntensity(_aoIntensity);

		if (_aoOnly)
		{
			_paramRender->SetEnable(true);
			
		}
		else
			_paramRender->SetEnable(false);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleSSAO>());

	return 0;
}