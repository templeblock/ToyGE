#include "SampleCommon.h"

using namespace ToyGE;

class SampleOIT : public SampleCommon
{
public:
	bool _enableOIT;
	Ptr<RenderMeshComponent> _objs;
	float _opacity;

	SampleOIT()
		: _enableOIT(true),
		_opacity(0.7f)
	{
		_sampleName = "OIT";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera = std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(float3(1.7f, 2.5f, -1.8f));
		camera->LookAt(float3(0.0f, 2.5f, 0.0f));

		//Init Scene
		auto scene = Global::GetScene();
		scene->SetAmbientColor(0.5f);

		//Add Objs
		{
			auto model = Asset::Find<MeshAsset>("Models/robot_clean_max/robot_clean.tmesh");
			if (!model->IsInit())
				model->Init();
			_objs = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 1.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			for (auto obj : _objs->GetSubRenderComponents())
			{
				obj->GetMeshElement()->GetMaterial()->SetTranslucent(true);
			}
		}

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "OIT");

		TwAddVarRW(_twBar, "EnableOIT", TW_TYPE_BOOLCPP, &_enableOIT, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "Opcacity", TW_TYPE_FLOAT, &_opacity, nullptr);
		TwSetParam(_twBar, "Opcacity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Opcacity", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Opcacity", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		/*auto renderConfig = _renderView->GetRenderConfig();
		if (_enableOIT)
			renderConfig->configMap["OIT"] = "true";
		else
			renderConfig->configMap["OIT"] = "false";*/
		if (_enableOIT)
			Global::GetRenderEngine()->GetSceneRenderer()->bOIT = true;
		else
			Global::GetRenderEngine()->GetSceneRenderer()->bOIT = false;

		for (auto obj : _objs->GetSubRenderComponents())
		{
			obj->GetMeshElement()->GetMaterial()->SetOpcacity(_opacity);

		}
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleOIT>());

	return 0;
}