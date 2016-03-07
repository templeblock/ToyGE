#include "SampleCommon.h"

using namespace ToyGE;

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
		_sampleName = "SSAO";
	}

	void Init() override
	{
		SampleCommon::Init();

		_renderView->sceneRenderingConfig.bReflectAmbientMap = false;
		_renderView->sceneRenderingConfig.bSSR = false;

		auto pp = std::make_shared<PostProcessing>();
		_ssao = std::make_shared<SSAO>();
		pp->AddRender(_ssao);
		pp->AddRender(std::make_shared<ToneMapping>());
		_paramRender = std::make_shared<SharedParamRender>();
		_paramRender->SetRenderParam("AmbientOcclusion");
		_paramRender->SetRenderParamColorWrite(COLOR_WRITE_R);
		pp->AddRender(_paramRender);

		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera = _renderView->GetCamera();
		camera->SetPos(float3(10.0f, 2.0f, 0.0f));
		camera->Yaw(PI_DIV2);

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto pointLight = LightActor::Create<PointLightComponent>(scene);
		pointLight->GetLight<PointLightComponent>()->SetPos(float3(0.0f, 3.0f, 0.0f));
		pointLight->GetLight<PointLightComponent>()->SetColor(1.0f);
		pointLight->GetLight<PointLightComponent>()->SetIntensity(30.0f);
		pointLight->GetLight<PointLightComponent>()->SetCastShadow(true);

		//Add Objs
		std::vector<Ptr<RenderComponent>> objs;

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/crytek-sponza/sponza.tmesh");
			model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.01f, 0.01f, 0.01f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			auto actor = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}


		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "SSAO");

		TwAddVarRW(_twBar, "EnableSSAO", TW_TYPE_BOOLCPP, &_enableSSAO, nullptr);

		TwAddVarRW(_twBar, "ShowAO", TW_TYPE_BOOLCPP, &_aoOnly, nullptr);

		float2 minMax = float2(0.0f, 0.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "AORadius", TW_TYPE_FLOAT, &_aoRadius, nullptr);
		TwSetParam(_twBar, "AORadius", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "AORadius", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.1f;
		TwAddVarRW(_twBar, "AOPower", TW_TYPE_FLOAT, &_aoPower, nullptr);
		TwSetParam(_twBar, "AOPower", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "AOPower", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.01f;
		TwAddVarRW(_twBar, "AOIntensity", TW_TYPE_FLOAT, &_aoIntensity, nullptr);
		TwSetParam(_twBar, "AOIntensity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
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