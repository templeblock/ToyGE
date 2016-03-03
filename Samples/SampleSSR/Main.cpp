#include "SampleCommon.h"

using namespace ToyGE;

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
		_ssrIntensity(0.8f)
	{
		_sampleName = "SSR";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		_ssr = std::make_shared<SSR>();
		pp->AddRender(_ssr);
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera = _renderView->GetCamera();
		camera->SetPos(float3(-14.0f, 1.0f, -1.0f));
		camera->LookAt(float3(0.0f, 0.0f, 0.0f));

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		{
			auto spotLight = LightActor::Create<SpotLightComponent>(scene);
			spotLight->GetLight<SpotLightComponent>()->SetPos(float3(2.0f, 0.2f, 0.0f));
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

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/dabrovic-sponza/sponza.tmesh");
			model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
		}

		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			auto actor = model->GetMesh()->AddInstanceToScene(scene, float3(-5.0f, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(0.0f);

			for (auto obj : actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		/*std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);

		model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(0.0f);

		for (auto obj : objs)
			obj->SetMaterial(mat);*/

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "SSR");

		TwAddVarRW(_twBar, "EnableSSR", TW_TYPE_BOOLCPP, &_enableSSR, nullptr);

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "SSRMaxRoughness", TW_TYPE_FLOAT, &_ssrMaxRoughness, nullptr);
		TwSetParam(_twBar, "SSRMaxRoughness", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "SSRMaxRoughness", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "SSRMaxRoughness", "step", TW_PARAM_FLOAT, 1, &step);

		step = 0.1f;
		TwAddVarRW(_twBar, "SSRIntensity", TW_TYPE_FLOAT, &_ssrIntensity, nullptr);
		TwSetParam(_twBar, "SSRIntensity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
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