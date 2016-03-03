#include "SampleCommon.h"

using namespace ToyGE;

class SampleLPV : public SampleCommon
{
public:
	bool _enableLPV;
	float _lpvStrength;
	int32_t _numPropagationItrs;
	bool _enableGeometryOcclusion;
	float3 _color;
	Ptr<Actor> _bunnyActor;
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

		Global::GetRenderEngine()->GetSceneRenderer()->bLPV = true;

		auto pp = std::make_shared<PostProcessing>();
		/*_lpvRender = std::make_shared<LPV>();
		pp->AddRender(_lpvRender);*/
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		//Init Scene
		auto scene = Global::GetScene();

		//Set Camera
		_renderView->GetCamera()->SetPos(float3(0.0f, 3.0f, 0.0f));
		_renderView->GetCamera()->Yaw(-PI_DIV2);

		//Add Light
		auto dirLight = LightActor::Create<DirectionalLightComponent>(scene);
		dirLight->GetLight<DirectionalLightComponent>()->SetDirection(_lightDir);
		dirLight->GetLight<DirectionalLightComponent>()->SetColor(1.0f);
		dirLight->GetLight<DirectionalLightComponent>()->SetIntensity(5.0f);
		dirLight->GetLight<DirectionalLightComponent>()->SetCastShadow(true);
		dirLight->GetLight<DirectionalLightComponent>()->SetCastLPV(true);

		_dirLight = dirLight->GetLight<DirectionalLightComponent>();

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			_bunnyActor = model->GetMesh()->AddInstanceToScene(scene, float3(-5.0f, 0.0f, 0.0f), float3(0.2f, 0.2f, 0.2f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(_color);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : _bunnyActor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
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

		Global::GetRenderEngine()->GetSceneRenderer()->bLPV = _enableLPV;

		if (!Global::GetRenderEngine()->GetSceneRenderer()->GetLPVRenderer())
			return;

		Global::GetRenderEngine()->GetSceneRenderer()->GetLPVRenderer()->SetLPVStrength(_lpvStrength);
		Global::GetRenderEngine()->GetSceneRenderer()->GetLPVRenderer()->SetNumPropagationItrs(_numPropagationItrs);
		Global::GetRenderEngine()->GetSceneRenderer()->GetLPVRenderer()->SetGeometryOcclusion(_enableGeometryOcclusion);

		_dirLight->SetDirection(_lightDir);

		for (auto obj : _bunnyActor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
			obj->GetMaterial()->SetBaseColor(_color);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleLPV>());

	return 0;
}