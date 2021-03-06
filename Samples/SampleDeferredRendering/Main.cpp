#include "SampleCommon.h"

using namespace ToyGE;

class SampleDeferredRendering : public SampleCommon
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
	Vector<ColorWriteMask, 4> _colorWriteMask;

	SampleDeferredRendering()
	{
		_sampleName = "DeferredRendering";

		_viewTextureType = NONE;

		_colorWriteMask = Vector<ColorWriteMask, 4>(COLOR_WRITE_R, COLOR_WRITE_G, COLOR_WRITE_B, COLOR_WRITE_A);
	}

	void Init() override
	{
		SampleCommon::Init();

		_renderView->sceneRenderingConfig.bGenVelocityMap = true;
		_renderView->sceneRenderingConfig.bReflectAmbientMap = false;
		_renderView->sceneRenderingConfig.bSSR = false;

		auto pp = std::make_shared<PostProcessing>();
		pp->AddRender(std::make_shared<MotionBlur>());
		pp->AddRender(std::make_shared<ToneMapping>());
		_paramRender = std::make_shared<SharedParamRender>();
		pp->AddRender(_paramRender);
		//pp->AddRender(std::make_shared<FXAA>());
		pp->AddRender(std::make_shared<TweakBarRenderer>());
		_renderView->SetPostProcessing(pp);

		_renderView->GetCamera()->Yaw(-PI_DIV2);

		//Init Scene
		auto scene = Global::GetScene();

		//scene->SetAmbientColor(0.02f);

		//Add Light
		auto pointLight = LightActor::Create<PointLightComponent>(scene);
		pointLight->GetLight<PointLightComponent>()->SetPos(float3(0.0f, 6.0f, 0.0f));
		pointLight->GetLight<PointLightComponent>()->SetColor(1.0f);
		pointLight->GetLight<PointLightComponent>()->SetIntensity(60.0f);
		pointLight->GetLight<PointLightComponent>()->SetCastShadow(true);

		std::vector<Ptr<RenderComponent>> objs;

		auto model = Asset::FindAndInit<MeshAsset>("Models/crytek-sponza/sponza.tmesh");
		model->GetMesh()->AddInstanceToScene(scene, float3(0.0f, 0.0f, 0.0f), float3(0.01f, 0.01f, 0.01f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f));

		auto reflectionMapCapture = std::make_shared<ReflectionMapCapture>();
		reflectionMapCapture->SetPos(float3(0.0f, 6.0f, 0.0f));
		reflectionMapCapture->SetRadius(40.0f);

		Global::GetScene()->AddReflectionMapCapture(reflectionMapCapture);
		Global::GetScene()->InitReflectionMaps();

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
			{ ZBUFFER, "SceneLinearClipDepth" },
			{ GBUFFER_0, "GBuffer0" },
			{ GBUFFER_1, "GBuffer1" },
			{ LIGHTING_0, "Lighting0" },
			{ LIGHTING_1, "Lighting1" }
		};

		_paramRender->SetRenderParam(renderParamMap[_viewTextureType]);
		if (_viewTextureType == GBUFFER_1)
		{
			_paramRender->SetRenderParamAsNormal(true);
			_paramRender->bDecode = true;
		}
		else
			_paramRender->SetRenderParamAsNormal(false);

		_paramRender->SetRenderParamColorWrite(_colorWriteMask);


		//_renderView->GetCamera()->Walk(-0.2f * elapsedTime);
	}
};

int main()
{
	SampleCommon::Start(std::make_shared<SampleDeferredRendering>());

	return 0;
}