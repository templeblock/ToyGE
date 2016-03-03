#include "SampleCommon.h"

using namespace ToyGE;

class SampleMotionBlur : public SampleCommon
{
public:
	Ptr<Actor> _actor;
	float _rotateSpeed;
	float _rotateAngle;
	bool _enableMB;
	Ptr<MotionBlur> _mb;

	SampleMotionBlur()
		: _rotateSpeed(PI2 / 0.2f),
		_rotateAngle(0.0f),
		_enableMB(true)
	{
		_sampleName = "MotionBlur";
	}

	void Init() override
	{
		SampleCommon::Init();

		auto pp = std::make_shared<PostProcessing>();
		_mb = std::make_shared<MotionBlur>();
		pp->AddRender(_mb);
		pp->AddRender(std::make_shared<ToneMapping>());
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		Global::GetRenderEngine()->GetSceneRenderer()->bGenVelocityMap = true;

		_renderView->GetCamera()->Pitch(PI_DIV2);

		//Init Scene
		auto scene = Global::GetScene();

		/*auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetRadiance(XMFLOAT3(30.0f, 30.0f, 30.0f));
		pointLightCom->SetCastShadow(true);
		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);*/

		auto model = Asset::FindAndInit<MeshAsset>("Models/stanford_bunny/stanford_bunny.tmesh");
		_actor = model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.1f, 0.1f, 0.1f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

		auto texAsset = Asset::FindAndInit<TextureAsset>("Textures/uffizi_cross.dds");
		auto reflectionMap = std::make_shared<ReflectionMap>();
		reflectionMap->SetEnvironmentMap(texAsset->GetTexture());
		reflectionMap->InitPreComputedData();

		auto mat = std::make_shared<Material>();
		mat->SetBaseColor(1.0f);
		mat->SetRoughness(0.0f);
		mat->SetMetallic(1.0f);

		for (auto & obj : _actor->GetRootTransformComponent()->Cast<RenderMeshComponent>()->GetSubRenderComponents())
		{
			obj->SetMaterial(mat);
			obj->SetReflectionMap(reflectionMap);
		}

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "MotionBlur");

		TwAddVarRW(_twBar, "EnabelMotionBlur", TW_TYPE_BOOLCPP, &_enableMB, nullptr);

		TwAddVarRW(_twBar, "RotateSpeed", TW_TYPE_FLOAT, &_rotateSpeed, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		//auto rotateOrienXM = rotation_axis(float3(0.0f, 1.0f, 0.0f), _rotateAngle);
		//XMFLOAT4 rotateOrien;
		//XMStoreFloat4(&rotateOrien, rotateOrienXM);

		_actor->GetRootTransformComponent()->SetOrientation(rotation_axis(float3(0.0f, 1.0f, 0.0f), _rotateAngle));

		_rotateAngle += _rotateSpeed * elapsedTime;

		_mb->SetEnable(_enableMB);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleMotionBlur>());

	return 0;
}
