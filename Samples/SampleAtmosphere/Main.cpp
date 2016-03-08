#include "SampleCommon.h"

using namespace ToyGE;

class SampleAtmosphere : public SampleCommon
{
public:
	Ptr<DirectionalLightComponent> _light;
	Ptr<PostProcessVolumetricLight> _ppvl;
	float3 _sunRadiance;
	float3 _sunDir;
	float _ppvlDensity;
	float _ppvlIntensity;
	float _ppvlDecay;

	SampleAtmosphere()
		: _sunRadiance(20.0f),
		_sunDir(0.0f, -0.02f, -1.0f),
		_ppvlDensity(0.8f),
		_ppvlIntensity(0.8f),
		_ppvlDecay(0.6f)
	{
		_sampleName = "Atmosphere";
	}

	void Init() override
	{
		SampleCommon::Init();

		//_renderView->sceneRenderingConfig.bRenderingAtmosphere = true;
		_renderView->sceneRenderingConfig.bSSR = false;
		auto pp = std::make_shared<PostProcessing>();

		_ppvl = std::make_shared<PostProcessVolumetricLight>();
		_ppvl->SetEnable(false);
		pp->AddRender(_ppvl);
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		auto camera =  std::static_pointer_cast<PerspectiveCamera>(_renderView->GetCamera());
		camera->SetPos(float3(-0.5f, 0.5f, -3.0f));
		//camera->LookTo(0.0f, float3(0.0f, 1.0f, 0.0), float3(0.0f, 0.0f, -1.0));
		camera->SetFar(1e4);

		//Init Scene
		auto scene = Global::GetScene();
		scene->SetAmbientColor(0.02f);
		scene->SetRenderSun(true);

		//Add Light
		auto dirLight = LightActor::Create<DirectionalLightComponent>(scene);
		dirLight->GetLight<DirectionalLightComponent>()->SetCastShadow(true);
		_light = dirLight->GetLight<DirectionalLightComponent>();

		//Global::GetRenderEngine()->GetSceneRenderer()->SetSunLight(dirLight->GetLight<DirectionalLightComponent>());
		_ppvl->SetLight(_light);

		//Add Objs
		{
			auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
			auto objs = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 1.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(1.0f);

			for (auto obj : objs->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
				obj->SetMaterial(mat);
		}

		{
			/*auto mat = std::make_shared<Material>();
			mat->SetBaseColor(1.0f);
			mat->SetRoughness(1.0f);
			mat->SetMetallic(0.0f);
			auto sphere = CommonMesh::CreateSphere(1.0f, 500);
			auto sphereObj = sphere->AddInstanceToScene(
				scene,
				float3(0.0f, -6370000.0f, 0.0f),
				float3(6370000.0f, 6370000.0f, 6370000.0f),
				Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
			for (auto & obj : sphereObj->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
			{
				obj->SetMaterial(mat);
				obj->SetCastShadows(false);
			}*/
		}

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "VolumetricLight");

		float2 minMax = float2(0.0f, 1.0f);
		float step = 0.01f;

		TwAddVarRW(_twBar, "SunDirection", TW_TYPE_DIR3F, &_sunDir, nullptr);

		/*TwAddVarRW(_twBar, "Density", TW_TYPE_FLOAT, &_ppvlDensity, nullptr);
		TwSetParam(_twBar, "Density", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Density", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Density", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Density", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Intensity", TW_TYPE_FLOAT, &_ppvlIntensity, nullptr);
		TwSetParam(_twBar, "Intensity", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Intensity", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Intensity", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "Decay", TW_TYPE_FLOAT, &_ppvlDecay, nullptr);
		TwSetParam(_twBar, "Decay", "group", TW_PARAM_CSTRING, 1, "PPVolumetricLight");
		TwSetParam(_twBar, "Decay", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "Decay", "max", TW_PARAM_FLOAT, 1, &minMax.y());
		TwSetParam(_twBar, "Decay", "step", TW_PARAM_FLOAT, 1, &step);*/
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		static float3 preSunRadiance = 0.0f;
		static float3 preSunDir = 0.0f;

		if (any(preSunRadiance != _sunRadiance) || any(preSunDir != _sunDir))
		{
			Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->SetSunRadiance(_sunRadiance);
			Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->SetSunDirection(_sunDir);

			Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->UpdateSunLight(_light);
			Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->UpdateAmbientAndReflectionMap(Global::GetScene());
			Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->RecomputeSunRenderColor();

			preSunRadiance = _sunRadiance;
			preSunDir = _sunDir;
		}

		_ppvl->SetDensity(_ppvlDensity);
		_ppvl->SetIntensity(_ppvlIntensity);
		_ppvl->SetDecay(_ppvlDecay);

		//auto texDesc = Global::GetScene()->GetAmbientMap()->->GetDesc();
		//texDesc.width = 2048;
		//texDesc.height = 2048;
		//texDesc.bCube = false;
		//texDesc.arraySize = 1;
		//texDesc.mipLevels = 1;
		//auto tmpTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		//auto tmpTex = tmpTexRef->Get()->Cast<Texture>();
		////CubeMapToHemiPnoramic(Global::GetScene()->GetAmbientTexture(), tmpTex);
		//Global::GetRenderEngine()->GetSceneRenderer()->GetAtmosphereRenderer()->RenderHemiPanoramicMap(tmpTex);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleAtmosphere>());

	return 0;
}