#include "SampleCommon.h"

using namespace ToyGE;

class SampleRefraction : public SampleCommon
{
public:
	Ptr<LightComponent> _light;
	Ptr<Material> _mat;
	float _refractionIndex;
	float _opacity;
	Ptr<Actor> _actor;

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
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		// Set Camera
		auto camera = std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(float3(0.0f, 1.0f, -2.0f));

		// Init Scene
		auto scene = Global::GetScene();

		// Add Light
		{
			auto spotLight = LightActor::Create<SpotLightComponent>(scene);
			spotLight->GetLight<SpotLightComponent>()->SetPos(float3(5.0f, 0.2f, 0.0f));
			spotLight->GetLight<SpotLightComponent>()->SetDirection(float3(-1.0f, -0.0f, 0.0f));
			spotLight->GetLight<SpotLightComponent>()->SetColor(float3(1.0f, 0.0f, 0.0f));
			spotLight->GetLight<SpotLightComponent>()->SetIntensity(5.0f);
			spotLight->GetLight<SpotLightComponent>()->SetDecreaseSpeed(50.0f);
			spotLight->GetLight<SpotLightComponent>()->SetCastShadow(true);
		}
		{
			auto pointLight = LightActor::Create<PointLightComponent>(scene);
			pointLight->GetLight<PointLightComponent>()->SetPos(float3(0.0f, 6.0f, -0.0f));
			pointLight->GetLight<PointLightComponent>()->SetColor(1.0f);
			pointLight->GetLight<PointLightComponent>()->SetIntensity(50.0f);
			pointLight->GetLight<PointLightComponent>()->SetCastShadow(true);
		};

		// Add Objs
		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_dragon/dragon.tmesh");
			_actor = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);
			mat->SetTranslucent(true);
			mat->SetRefraction(true);
			mat->SetOpcacity(_opacity);
			mat->SetRefractionIndex(_refractionIndex);
			_mat = mat;

			for (auto obj : _actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
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