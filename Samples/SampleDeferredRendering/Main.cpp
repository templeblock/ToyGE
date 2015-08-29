#include "SampleCommon.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"

using namespace ToyGE;
using namespace XNA;

class DeferredRenderingSample : public SampleCommon
{
public:
	enum DeferredViewTextureType
	{
		NONE,
		ZBUFFER,
		GBUFFER_0,
		GBUFFER_1,
		LIGHTING_0,
		LIGHTING_1
	};

	DeferredViewTextureType _viewTextureType;
	Ptr<SharedParamRender> _paramRender;
	Vector4<ColorWriteMask> _colorWriteMask;

	DeferredRenderingSample()
	{
		_sampleName = L"DeferredRendering";

		_viewTextureType = NONE;

		_colorWriteMask = Vector4<ColorWriteMask>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A);
	}

	void Startup() override
	{
		SampleCommon::Startup();

		_renderView->AddPostProcessRender(std::make_shared<HDR>());
		_paramRender = std::make_shared<SharedParamRender>();
		_renderView->AddPostProcessRender(_paramRender);
		_renderView->AddPostProcessRender(std::make_shared<GammaCorrection>());
		_renderView->AddPostProcessRender(std::make_shared<FXAA>());
		_renderView->AddPostProcessRender(std::make_shared<TweakBarRenderer>());

		//Init Scene
		auto scene = Global::GetScene();

		//Add Light
		auto pointLightCom = std::make_shared<PointLightComponent>();
		pointLightCom->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
		pointLightCom->SetRadiance(XMFLOAT3(30.0f, 30.0f, 30.0f));
		pointLightCom->SetCastShadow(true);
		auto pointLightObj = std::make_shared<SceneObject>();
		pointLightObj->AddComponent(pointLightCom);
		pointLightObj->ActiveAllComponents();
		scene->AddSceneObject(pointLightObj);

		std::vector<Ptr<RenderComponent>> objs;

		auto model = Global::GetResourceManager(RESOURCE_MODEL)->As<ModelManager>()->AcquireResource(L"crytek-sponza/sponza.tx");
		model->AddInstanceToScene(scene, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), nullptr);


		//Init UI
		TwSetParam(_twBar, nullptr, "label", TW_PARAM_CSTRING, 1, "DeferredRendering");

		TwEnumVal bufferTypeEnumVals[] = 
		{
			{ NONE, "None" },
			{ ZBUFFER, "ZBuffer" },
			{ GBUFFER_0, "Gbuffer0" },
			{ GBUFFER_1, "Gbuffer1" },
			{ LIGHTING_0, "Lighting0" },
			{ LIGHTING_1, "Lighting1" }
		};

		auto twEnumType = TwDefineEnum("DeferredViewTextureType", bufferTypeEnumVals, _countof(bufferTypeEnumVals));
		TwAddVarRW(_twBar, "ShowBuffer", twEnumType, &_viewTextureType, nullptr);


		TwEnumVal colorWriteEnumVals[] = 
		{
			{ COLOR_WRITE_R, "R" },
			{ COLOR_WRITE_G, "G" },
			{ COLOR_WRITE_B, "B" },
			{ COLOR_WRITE_A, "A" },
		};

		twEnumType = TwDefineEnum("ColorWriteMask", colorWriteEnumVals, _countof(colorWriteEnumVals));

		String group = "ColorWriteMask";

		TwAddVarRW(_twBar, "ColorChannel0", twEnumType, &_colorWriteMask[0], nullptr);
		TwSetParam(_twBar, "ColorChannel0", "group", TW_PARAM_CSTRING, 1, group.c_str());

		TwAddVarRW(_twBar, "ColorChannel1", twEnumType, &_colorWriteMask[1], nullptr);
		TwSetParam(_twBar, "ColorChannel1", "group", TW_PARAM_CSTRING, 1, group.c_str());

		TwAddVarRW(_twBar, "ColorChannel2", twEnumType, &_colorWriteMask[2], nullptr);
		TwSetParam(_twBar, "ColorChannel2", "group", TW_PARAM_CSTRING, 1, group.c_str());

		TwAddVarRW(_twBar, "ColorChannel3", twEnumType, &_colorWriteMask[3], nullptr);
		TwSetParam(_twBar, "ColorChannel3", "group", TW_PARAM_CSTRING, 1, group.c_str());
	}

	void Update(float elapsedTime) override
	{
		SampleCommon::Update(elapsedTime);

		static std::map<DeferredViewTextureType, String> renderParamMap = 
		{
			{ NONE, "" },
			{ ZBUFFER, CommonRenderShareName::RawDepth() },
			{ GBUFFER_0, CommonRenderShareName::GBuffer(0) },
			{ GBUFFER_1, CommonRenderShareName::GBuffer(1) },
			{ LIGHTING_0, CommonRenderShareName::Lighting(0) },
			{ LIGHTING_1, CommonRenderShareName::Lighting(1) }
		};

		_paramRender->SetRenderParam(renderParamMap[_viewTextureType]);
		if (_viewTextureType == GBUFFER_1)
			_paramRender->SetRenderParamAsNormal(true);
		else
			_paramRender->SetRenderParamAsNormal(false);

		_paramRender->SetRenderParamColorWrite(_colorWriteMask);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<DeferredRenderingSample>());

	return 0;
}