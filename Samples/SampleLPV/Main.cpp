#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleLPV : public SampleCommon
{
public:
	bool _enableLPV;
	Ptr<LPV> _lpvRender;
	float _lpvStrength;
	int32_t _numPropagationItrs;
	bool _enableGeometryOcclusion;
	float3 _color;
	Ptr<RenderMeshComponent> _meshObj;
	float3 _lightDir = float3(1.0f, -8.0f, 1.0f);
	Ptr<DirectionalLightComponent> _dirLight;

	SampleLPV()
		: _enableLPV(true),
		_lpvStrength(1.0f),
		_color(1.0f, 0.0f, 0.0f),
		_numPropagationItrs(8),
		_enableGeometryOcclusion(true)
	{
		_sampleName = "LPV";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		_lpvRender = std::make_shared<LPV>();
		pp->AddRender(_lpvRender);
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		//Init Scene
		auto scene = Global::GetScene();

		//Set Camera
		_renderView->GetCamera()->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		_renderView->GetCamera()->Yaw(-XM_PIDIV2);

		//Add Light
		auto dirLightCom = std::make_shared<DirectionalLightComponent>();
		dirLightCom->SetDirection(XMFLOAT3(_lightDir.x(), _lightDir.y(), _lightDir.z()));
		dirLightCom->SetColor(1.0f);
		dirLightCom->SetIntensity(8.0f);
		dirLightCom->SetCastShadow(true);
		dirLightCom->SetCastLPV(true);
		auto dirLightObj = std::make_shared<SceneObject>();
		dirLightObj->AddComponent(dirLightCom);
		dirLightObj->ActiveAllComponents();
		scene->AddSceneObject(dirLightObj);

		_dirLight = dirLightCom;

		{
			auto model = Asset::Find<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			if (!model->IsInit())
				model->Init();
			model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::Find<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			if (!model->IsInit())
				model->Init();
			_meshObj = model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(-5.0f, 0.0f, 0.0f), XMFLOAT3(0.2f, 0.2f, 0.2f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(_color);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : _meshObj->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		/*std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);
*/
		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "LPV");

		TwAddVarRW(_twBar, "EnableLPV", TW_TYPE_BOOLCPP, &_enableLPV, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "Strength", TW_TYPE_FLOAT, &_lpvStrength, nullptr);
		TwSetParam(_twBar, "Strength", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Strength", "step", TW_PARAM_FLOAT, 1, &step);

		int2 minMax_i = int2(0, 10);
		int32_t step_i = 1;

		TwAddVarRW(_twBar, "NumItrs", TW_TYPE_INT32, &_numPropagationItrs, nullptr);
		TwSetParam(_twBar, "NumItrs", "min", TW_PARAM_INT32, 1, &minMax_i.x());
		TwSetParam(_twBar, "NumItrs", "max", TW_PARAM_INT32, 1, &minMax_i.y());
		TwSetParam(_twBar, "NumItrs", "step", TW_PARAM_INT32, 1, &step_i);

		TwAddVarRW(_twBar, "Color", TW_TYPE_COLOR3F, &_color, nullptr);

		TwAddVarRW(_twBar, "LightDirection", TW_TYPE_DIR3F, &_lightDir, nullptr);

		TwAddVarRW(_twBar, "EnableGeometryOcclusion", TW_TYPE_BOOLCPP, &_enableGeometryOcclusion, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_lpvRender->SetEnable(_enableLPV);

		_lpvRender->SetLPVStrength(_lpvStrength);
		_lpvRender->SetNumPropagationItrs(_numPropagationItrs);
		_lpvRender->SetGeometryOcclusion(_enableGeometryOcclusion);

		_dirLight->SetDirection(XMFLOAT3(_lightDir.x(), _lightDir.y(), _lightDir.z()));

		for (auto obj : _meshObj->GetSubRenderComponents())
			obj->GetMaterial()->SetBaseColor(_color);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleLPV>());

	return 0;
}