#include "SampleCommon.h"

using namespace ToyGE;

class SampleDepthOfField : public SampleCommon
{
public:
	Ptr<BokehDepthOfField> _dof;
	bool _enableDof;
	//Ptr<PhysicalCamera> _camera;
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
		_sampleName = "DepthOfField";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		_dof = std::make_shared<BokehDepthOfField>();
		pp->AddRender(_dof);
		pp->AddRender(std::make_shared<HDR>());
		pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		//_camera = std::static_pointer_cast<PhysicalCamera>(_renderView->GetCamera());

		//Init Scene
		auto scene = Global::GetScene();

		auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(float3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetColor(1.0f);
		pointLightCom->SetIntensity(60.0f);

		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);

		auto texAsset = Asset::Find<TextureAsset>("Textures/uffizi_cross.dds");
		if (!texAsset->IsInit())
			texAsset->Init();
		auto reflectionMap = std::make_shared<ReflectionMap>();
		reflectionMap->SetEnvironmentMap(texAsset->GetTexture());
		reflectionMap->InitPreComputedData();

		/*auto backgroundRender = std::static_pointer_cast<SkyBox>(Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->GetBackgroundRender());
		auto bkTex = backgroundRender->GetTexture();

		auto reflecMap = ReflectionMap::Create();
		reflecMap->SetEnvMap(bkTex);
		reflecMap->InitPreComputedData();*/

		auto model = Asset::Find<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
		if (!model->IsInit())
			model->Init();

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

		for (int i = 0; i < _countof(posX); ++i)
		{
			float x = posX[i];

			auto objs = model->GetMesh()->AddInstanceToScene(scene, float3(x, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

			auto mat = std::make_shared<Material>();
			mat->SetBaseColor(color[i]);
			mat->SetRoughness(0.0f);
			mat->SetMetallic(1.0f);

			for (auto & obj : objs->GetSubRenderComponents())
			{
				obj->SetReflectionMap(reflectionMap);
				obj->SetMaterial(mat);
			}
		}

		_renderView->GetCamera()->SetPos(float3(8.0f, 0.8f, -2.0f));
		_renderView->GetCamera()->LookAt(float3(0.0f, 0.8f, 0.0f));
		_renderView->GetCamera()->Cast<PerspectiveCamera>()->SetFovAngle(PI / 10.0f);
		/*_renderView->GetCamera()->SetFocalLength(100.0f);
		_renderView->GetCamera()->SetFocalDistance(8000.0f);
		_renderView->GetCamera()->SetFarPlane(1500.0f);*/

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
		TwSetParam(_twBar, "FocalDistance", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "FocalDistance", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "FocalAreaLength", TW_TYPE_FLOAT, &_focalAreaLength, nullptr);
		TwSetParam(_twBar, "FocalAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "FocalAreaLength", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "NearAreaLength", TW_TYPE_FLOAT, &_nearAreaLength, nullptr);
		TwSetParam(_twBar, "NearAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x());
		TwSetParam(_twBar, "NearAreaLength", "step", TW_PARAM_FLOAT, 1, &step);

		TwAddVarRW(_twBar, "FarAreaLength", TW_TYPE_FLOAT, &_farAreaLength, nullptr);
		TwSetParam(_twBar, "FarAreaLength", "min", TW_PARAM_FLOAT, 1, &minMax.x());
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