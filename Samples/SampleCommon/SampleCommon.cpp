#include "SampleCommon.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

using namespace ToyGE;

DECLARE_SHADER(, RenderNormalPS, SHADER_PS, "RenderNormal", "RenderNormalPS", SM_4);

SharedParamRender::SharedParamRender()
	: _renderParamColorWrite(COLOR_WRITE_ALL)
{

}

void SharedParamRender::Render(const Ptr<RenderView> & view)
{
	if (_renderParam.size() > 0)
	{
		auto param = view->GetViewRenderContext()->GetSharedTexture(_renderParam);
		if (!param)
			return;

		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		if (_renderParamAsNormal)
		{
			auto ps = Shader::FindOrCreate<RenderNormalPS>();
			ps->SetScalar("bDecode", (int)bDecode);
			ps->SetSRV("srcTex", param->GetShaderResourceView(0, 0, 0, 0, false, _format));
			ps->SetSampler("linearSampler", SamplerTemplate<>::Get());
			ps->Flush();
			DrawQuad({ sceneTex->GetRenderTargetView(0 ,0, 1) });
		}
		else
		{
			ToyGE::Transform(
				param->GetShaderResourceView(0, 0, 0, 0, false, _format),
				sceneTex->GetRenderTargetView(0, 0, 1),
				_renderParamColorWrite);
		}
	}
}


void SampleCommon::Start(const ToyGE::Ptr<App> & app)
{
	ToyGE::EngineDriver::Init(app);
	ToyGE::EngineDriver::Run();
}

SampleCommon::SampleCommon()
	: _cameraMoveSpeed(5.0f),
	_cameraRotateSpeed(0.25f),
	_bMouseDown(false)
{

}

void SampleCommon::Init()
{
	Global::GetWindow()->SetTitle(_sampleName);

	//Create Scene
	auto scene = std::make_shared<Scene>();
	Global::SetScene(scene);
	scene->SetAmbientColor(0.05f);

	//Create View
	_renderView = std::make_shared<RenderView>();
	scene->AddView(_renderView);

	//_renderView->SetRenderConfig(std::make_shared<RenderConfig>());

	//Set Resize Events
	Global::GetWindow()->OnResizeEvent().connect(
		[&](const Ptr<Window> & window, int32_t preWidth, int32_t preHeight)
	{
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(Global::GetWindow()->Width());
		vp.height = static_cast<float>(Global::GetWindow()->Height());
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		_renderView->SetViewport(vp);

		_renderView->GetCamera()->Cast<PerspectiveCamera>()->SetAspectRatio(vp.width / vp.height);

		_renderView->SetRenderTarget(Global::GetRenderEngine()->GetFrameBuffer()->GetRenderTargetView(0, 0, 1));
	});


	//Create Viewport
	RenderViewport vp;
	vp.topLeftX = 0.0f;
	vp.topLeftY = 0.0f;
	vp.width = static_cast<float>(Global::GetWindow()->Width());
	vp.height = static_cast<float>(Global::GetWindow()->Height());
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	_renderView->SetViewport(vp);

	_renderView->SetRenderTarget(Global::GetRenderEngine()->GetFrameBuffer()->GetRenderTargetView(0, 0, 1));

	//Create Camera
	auto camera = std::make_shared<PerspectiveCamera>(PI_DIV2, vp.width / vp.height, 0.1f, 1e+2f);
	camera->SetPos(float3(0.0f, 3.0f, 0.0f));
	_renderView->SetCamera(camera);
	FrameInfoRender::traceCamera = camera;

	//Set Background Render
	auto skyboxTex = Asset::Find<TextureAsset>("Textures/uffizi_cross.dds");
	skyboxTex->Init();
	scene->SetAmbientTexture(skyboxTex->GetTexture());

	//UI
	_twBar = TwNewBar("Sample");

	int size[2] = { 200, 400 };
	TwSetParam(_twBar, nullptr, "size", TW_PARAM_INT32, 2, size);

	int pos[2] = { Global::GetWindow()->Width() - size[0], 10 };
	TwSetParam(_twBar, nullptr, "position", TW_PARAM_INT32, 2, pos);


	//Init Inputs
	auto numMouses = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_MOUSE);
	for (int32_t i = 0; i < numMouses; ++i)
	{
		auto mouse = std::static_pointer_cast<InputMouse>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_MOUSE, i));
		mouse->OnButtonDownEvent().connect(std::bind(&SampleCommon::MouseDown, this, std::placeholders::_1, std::placeholders::_2));
		mouse->OnButtonUpEvent().connect(std::bind(&SampleCommon::MouseUp, this, std::placeholders::_1, std::placeholders::_2));
		mouse->OnMouseMoveEvent().connect(std::bind(&SampleCommon::MouseMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	auto numKeyboards = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_KEYBORD);
	for (int32_t i = 0; i < numKeyboards; ++i)
	{
		auto keyboard = std::static_pointer_cast<InputKeyboard>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_KEYBORD, i));
		keyboard->KeyUpEvent().connect(std::bind(&SampleCommon::KeyUp, this, std::placeholders::_1, std::placeholders::_2));
	}
};

void SampleCommon::Update(float elapsedTime)
{
	float dst = elapsedTime * _cameraMoveSpeed;

	if (InputKeyboard::ExistKeyDown(KEY_W))
		_renderView->GetCamera()->Walk(dst);
	if (InputKeyboard::ExistKeyDown(KEY_S))
		_renderView->GetCamera()->Walk(-dst);
	if (InputKeyboard::ExistKeyDown(KEY_A))
		_renderView->GetCamera()->Strafe(-dst);
	if (InputKeyboard::ExistKeyDown(KEY_D))
		_renderView->GetCamera()->Strafe(dst);
	if (InputKeyboard::ExistKeyDown(KEY_Q))
		_renderView->GetCamera()->Fly(dst);
	if (InputKeyboard::ExistKeyDown(KEY_E))
		_renderView->GetCamera()->Fly(-dst);
};

void SampleCommon::KeyUp(const Ptr<InputKeyboard> & keyboard, KeyCode code)
{
	if (code == KEY_Escape)
		Global::GetLooper()->SetExit(true);

	if (code == KEY_F1)
		Global::GetRenderEngine()->SetFullScreen(!Global::GetRenderEngine()->IsFullScreen());
}


void SampleCommon::MouseDown(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button)
{
	if (MOUSE_BUTTON_R == button && IsPosInWindow(mouse->PosWindow()))
		_bMouseDown = true;
}

void SampleCommon::MouseUp(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, const ToyGE::MouseButton button)
{
	if (MOUSE_BUTTON_R == button)
		_bMouseDown = false;
}

void SampleCommon::MouseMove(const ToyGE::Ptr<ToyGE::InputMouse> & mouse, int relativeX, int relativeY)
{
	if (_bMouseDown)
	{
		float angleX = _cameraRotateSpeed * deg2rad((float)(relativeX));
		float angleY = _cameraRotateSpeed * deg2rad((float)(relativeY));

		_renderView->GetCamera()->Rotate(float3(0.0f, 1.0f, 0.0f), angleX);
		_renderView->GetCamera()->Pitch(angleY);
	}
}

bool SampleCommon::IsPosInWindow(const ToyGE::int2 & pos) const
{
	return pos.x() >= 0 && pos.x() <= Global::GetWindow()->Width()
		&& pos.y() >= 0 && pos.y() <= Global::GetWindow()->Height();
}