#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleDepthOfField : public SampleCommon
{
public:
	Ptr<BokehDepthOfField> _dof;
	bool _enableDof;
	Ptr<PhysicalCamera> _camera;
	float _focalDistance;
	float _focalAreaLength;
	float _nearAreaLength;
	float _farAreaLength;

	SampleDepthOfField()
		: _enableDof(true),
		_focalDistance(8.0f),
		_focalAreaLength(2.0f),
		_nearAreaLength(3.0f),
		_farAreaLength(5.0f)
	{
		_sampleName = L"DepthOfField";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_dof = std::make_shared<BokehDepthOfField>();
		_renderView->AddPostProcessRender(_dof);
		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		_camera = std::static_pointer_cast<PhysicalCamera>(_renderView->GetCamera());

		//Init Scene
		auto scene = Global::GetScene();

		auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetRadiance(XMFLOAT3(30.0f, 30.0f, 30.0f));
		pointLightCom->SetCastShadow(true);
		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);

		auto backgroundRender = std::static_pointer_cast<SkyBox>(Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->GetBackgroundRender());
		auto bkTex = backgroundRender->GetTexture();

		auto reflecMap = ReflectionMap::Create();
		reflecMap->SetEnvMap(bkTex);
		reflecMap->InitPreComputedData();

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");

		float3 color[] = 
		{
			float3(0.8f, 1.0f, 0.4f),
			float3(0.5f, 0.4f, 0.7f),
			float3(1.0f, 0.2f, 0.2f),
			float3(0.2f, 1.0f, 0.2f),
			float3(0.2f, 0.2f, 1.0f),

			float3(1.0f, 1.0f, 0.4f),

			float3(1.0f, 1.0f, 1.0f),
			float3(1.0f, 0.4f, 1.0f),
			float3(0.4f, 1.0f, 1.0f)
		};

		float posX[] =
		{
			-10.0f,
			-8.0f,
			-6.0f,
			-4.0f,
			-2.0f,

			0.0f,

			2.0f,
			4.0f,
			6.0f
		};

		for (int i = 0; i <= _countof(posX); ++i)
		{
			float x = posX[i];

			std::vector<Ptr<RenderComponent>> objs;
			model->AddInstanceToScene(scene, XMFLOAT3(x, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &objs);

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(color[i]);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(1.0f);

			for (auto & obj : objs)
			{
				obj->SetReflectionMap(reflecMap);
				obj->SetMaterial(mat);
			}
		}

		_camera->SetPos(XMFLOAT3(8.0f, 0.8f, 2.0f));
		_camera->LookAt(XMFLOAT3(0.0f, 0.8f, 0.0f));
		_camera->SetFocalLength(100.0f);
		_camera->SetFocalDistance(8000.0f);
		_camera->SetFarPlane(1500.0f);

		_dof->SetFocalDistance(_focalDistance);
		_dof->SetFocalAreaLength(_focalAreaLength);
		_dof->SetNearAreaLength(_nearAreaLength);
		_dof->SetFarAreaLength(_farAreaLength);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "DepthOfField");

		TwAddVarRW(_twBar, "EnabelDepthOfField", TW_TYPE_BOOLCPP, &_enableDof, nullptr);

		float2 minMax = float2(0.0f, 0.0f);
		float step = 0.1f;

		TwAddVarRW(_twBar, "FocalDistance", TW_TYPE_FLOAT, &_focalDistance, nullptr);
		TwSetParam(_twBar, "FocalDistance", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "FocalDistance", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "FocalAreaLength", TW_TYPE_FLOAT, &_focalAreaLength, nullptr);
		TwSetParam(_twBar, "FocalAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "FocalAreaLength", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "NearAreaLength", TW_TYPE_FLOAT, &_nearAreaLength, nullptr);
		TwSetParam(_twBar, "NearAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "NearAreaLength", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "FarAreaLength", TW_TYPE_FLOAT, &_farAreaLength, nullptr);
		TwSetParam(_twBar, "FarAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x);
		TwSetParam(_twBar, "FarAreaLength", "step", TW_PARAM_FLOAT, 1, &step);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		_dof->SetEnable(_enableDof);

		_dof->SetFocalDistance(_focalDistance);
		_dof->SetFocalAreaLength(_focalAreaLength);
		_dof->SetNearAreaLength(_nearAreaLength);
		_dof->SetFarAreaLength(_farAreaLength);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleDepthOfField>());

	return 0;
}