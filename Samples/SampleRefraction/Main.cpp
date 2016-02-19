#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleRefraction : public SampleCommon
{
public:
	Ptr<LightComponent> _light;
	Ptr<Material> _mat;
	float _refractionIndex;
	float _opacity;
	Ptr<RenderMeshComponent> _meshObj;

	SampleRefraction()
		: 
		_refractionIndex(1.5f),
		_opacity(0.0f)
	{
		_sampleName = "Refraction";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		// Set Camera
		auto camera = std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(XMFLOAT3(0.0f, 1.0f, -2.0f));

		// Init Scene
		auto scene = Global::GetScene();

		// Add Light
		{
			auto spotLightCom = std::make_shared<SpotLightComponent>();
			spotLightCom->SetPos(XMFLOAT3(5.0f, 0.2f, 0.0f));
			spotLightCom->SetDirection(XMFLOAT3(-1.0f, -0.0f, 0.0f));
			spotLightCom->SetColor(float3(1.0f, 0.0f, 0.0f));
			spotLightCom->SetIntensity(5.0f);
			spotLightCom->SetDecreaseSpeed(50.0f);
			spotLightCom->SetCastShadow(true);
			auto spotLightObj = std::make_shared<SceneObject>();
			spotLightObj->AddComponent(spotLightCom);
			spotLightObj->ActiveAllComponents();
		}
		{
			auto pointLightCom = std::make_shared<PointLightComponent>();
			pointLightCom->SetPos(XMFLOAT3(0.0f, 6.0f, -0.0f));
			pointLightCom->SetColor(1.0f);
			pointLightCom->SetIntensity(50.0f);
			pointLightCom->SetCastShadow(true);
			auto pointLightObj = std::make_shared<SceneObject>();
			pointLightObj->AddComponent(pointLightCom);
			pointLightObj->ActiveAllComponents();
			scene->AddSceneObject(pointLightObj);
		};

		// Add Objs
		{
			auto model = Asset::Find<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			if (!model->IsInit())
				model->Init();
			model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::Find<MeshAsset>("Models/stanford_dragon/dragon.tmesh");
			if (!model->IsInit())
				model->Init();
			_meshObj = model->GetMesh()->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);
			mat->SetTranslucent(true);
			mat->SetRefraction(true);
			mat->SetOpcacity(_opacity);
			mat->SetRefractionIndex(_refractionIndex);
			_mat = mat;

			for (auto obj : _meshObj->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		// Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "Refraction");

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		minMax.x() = 1.0f;
		TwAddVarRW(_twBar, "RefractionIndex", TW_TYPE_FLOAT, &_refractionIndex, nullptr);
		TwSetParam(_twBar, "RefractionIndex", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "RefractionIndex", "step", TW_PARAM_FLOAT, 1, &step);

		minMax.x() = 0.0f;
		TwAddVarRW(_twBar, "Opacity", TW_TYPE_FLOAT, &_opacity, nullptr);
		TwSetParam(_twBar, "Opacity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Opacity", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Opacity", "step", TW_PARAM_FLOAT, 1, &step);

	}
	
	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_mat->SetRefractionIndex(_refractionIndex);
		_mat->SetOpcacity(_opacity);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleRefraction>());

	return 0;
}