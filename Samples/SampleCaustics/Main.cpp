#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleCaustics : public SampleCommon
{
public:
	Ptr<LightComponent> _light;
	Ptr<Material> _mat;
	bool _enableCaustics;
	float _pointSize;
	float _refractionIndex;
	float _opacity;
	bool _dualRefraction;

	SampleCaustics()
		: _enableCaustics(true),
		_pointSize(0.1f),
		_refractionIndex(1.5f),
		_opacity(0.0f),
		_dualRefraction(false)
	{
		_sampleName = L"Caustics";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto renderConfig = std::make_shared<RenderConfig>();
		renderConfig->configMap["CausticsPointSize"] = std::to_string(_pointSize);

		_renderView->SetRenderConfig(renderConfig);

		//Set Camera
		auto camera = std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(XMFLOAT3(0.6f, 2.1f, 1.3f));
		camera->LookAt(XMFLOAT3(0.0f, 1.0f, 0.0f));

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetRadiance(XMFLOAT3(50.0f, 50.0f, 50.0f));
		pointLightCom->SetCastShadow(true);
		pointLightCom->SetCastCaustics(true);
		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);
		_light = pointLightCom;

		//Add Objs
		std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);
		mat->SetTranslucent(true);
		mat->SetRefraction(true);
		mat->SetOpacity(_opacity);
		mat->SetRefractionIndex(_refractionIndex);
		_mat = mat;
		for (auto obj : objs)
		{
			obj->SetMaterial(mat);
		}

		auto planeMesh = CommonMesh::CreatePlane(5.0f, 5.0f, 1, 1);
		auto planeObj = planeMesh->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		
		mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(1.0f);
		mat->SetMetallic(0.0f);
		mat->AddTexture(MATERIAL_TEXTURE_BASECOLOR, L"rocks.dds", 0);
		mat->AddTexture(MATERIAL_TEXTURE_BUMP, L"rocks_height_bump.dds", 0);
		mat->SetPOM(true);
		planeObj->SetMaterial(mat);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "Caustics");

		TwAddVarRW(_twBar, "EnableCaustics", TW_TYPE_BOOLCPP, &_enableCaustics, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "PointSize", TW_TYPE_FLOAT, &_pointSize, nullptr);
		TwSetParam(_twBar, "PointSize", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "PointSize", "step", TW_PARAM_FLOAT, 1, &step);

		minMax.x = 1.0f;
		TwAddVarRW(_twBar, "RefractionIndex", TW_TYPE_FLOAT, &_refractionIndex, nullptr);
		TwSetParam(_twBar, "RefractionIndex", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "RefractionIndex", "step", TW_PARAM_FLOAT, 1, &step);

		minMax.x = 0.0f;
		TwAddVarRW(_twBar, "Opacity", TW_TYPE_FLOAT, &_opacity, nullptr);
		TwSetParam(_twBar, "Opacity", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Opacity", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "Opacity", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "DualRefraction", TW_TYPE_BOOLCPP, &_dualRefraction, nullptr);
	}
	
	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_light->SetCastCaustics(_enableCaustics);

		auto renderConfig = _renderView->GetRenderConfig();
		renderConfig->configMap["CausticsPointSize"] = std::to_string(_pointSize);

		_mat->SetRefractionIndex(_refractionIndex);
		_mat->SetOpacity(_opacity);
		_mat->SetDualFace(_dualRefraction);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleCaustics>());

	return 0;
}