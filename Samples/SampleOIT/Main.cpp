#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleOIT : public SampleCommon
{
public:
	bool _enableOIT;
	std::vector<Ptr<RenderComponent>> _objs;
	float _opacity;

	SampleOIT()
		: _enableOIT(true),
		_opacity(0.7f)
	{
		_sampleName = L"OIT";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto renderConfig = std::make_shared<RenderConfig>();
		renderConfig->configMap["OIT"] = "true";
		_renderView->SetRenderConfig(renderConfig);

		auto camera = std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(XMFLOAT3(1.7f, 2.5f, -1.8f));
		camera->LookAt(XMFLOAT3(0.0f, 2.5f, 0.0f));

		//Init Scene
		auto scene = Global::GetScene();
		scene->SetAmbientColor(0.5f);

		//Add Objs
		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"robot_clean_max/robot_clean.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &_objs);

		for (auto obj : _objs)
		{
			auto mat = obj->GetMaterial();
			mat->SetTranslucent(true);
			mat->SetOpacity(_opacity);
		}

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "OIT");

		TwAddVarRW(_twBar, "EnableOIT", TW_TYPE_BOOLCPP, &_enableOIT, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "Opcacity", TW_TYPE_FLOAT, &_opacity, nullptr);
		TwSetParam(_twBar, "Opcacity", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "Opcacity", "max", TW_PARAM_FLOAT, 1, &minMax.y);
		TwSetParam(_twBar, "Opcacity", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		auto renderConfig = _renderView->GetRenderConfig();
		if (_enableOIT)
			renderConfig->configMap["OIT"] = "true";
		else
			renderConfig->configMap["OIT"] = "false";

		for (auto obj : _objs)
		{
			auto mat = obj->GetMaterial();
			mat->SetOpacity(_opacity);
		}
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleOIT>());

	return 0;
}