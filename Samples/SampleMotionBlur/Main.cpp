#include "SampleCommon.h"

using namespace ToyGE;
using namespace XNA;

class SampleMotionBlur : public SampleCommon
{
public:
	std::vector<Ptr<RenderComponent>> _objs;
	float _rotateSpeed;
	float _rotateAngle;
	bool _enableMB;
	Ptr<MotionBlur> _mb;

	SampleMotionBlur()
		: _rotateSpeed(XM_2PI / 0.2f),
		_rotateAngle(0.0f),
		_enableMB(true)
	{
		_sampleName = L"MotionBlur";
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_mb = std::make_shared<MotionBlur>();
		_renderView->AddPostProcessRender(_mb);
		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		auto renderConfig = std::make_shared<RenderConfig>();
		renderConfig->configMap["GenMotionVelocityMap"] = "true";

		_renderView->SetRenderConfig(renderConfig);

		_renderView->GetCamera()->Pitch(XM_PIDIV2);

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

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"stanford_bunny.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 0.1f, 0.1f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), &_objs);

		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "MotionBlur");

		TwAddVarRW(_twBar, "EnabelMotionBlur", TW_TYPE_BOOLCPP, &_enableMB, nullptr);

		TwAddVarRW(_twBar, "RotateSpeed", TW_TYPE_FLOAT, &_rotateSpeed, nullptr);
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		auto rotateOrienXM = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), _rotateAngle);
		XMFLOAT4 rotateOrien;
		XMStoreFloat4(&rotateOrien, rotateOrienXM);

		for (auto & obj : _objs)
		{
			obj->SetOrientation(rotateOrien);
			obj->UpdateTransform();
		}

		_rotateAngle += _rotateSpeed * elapsedTime;

		_mb->SetEnable(_enableMB);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleMotionBlur>());

	return 0;
}
