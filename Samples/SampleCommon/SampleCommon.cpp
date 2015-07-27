#include "SampleCommon.h"

using namespace ToyGE;

void SampleCommon::Start(const ToyGE::Ptr<App> & app)
{
	File::SetCurrentPathToProgram();

	Config config;
	bool configLoad = Config::Load(L"../../../Media/Config.xml", config);

	ToyGE::EngineDriver::StartUp(config, app);

	ToyGE::EngineDriver::Run();
}

SampleCommon::SampleCommon()
	: _cameraMoveSpeed(5.0f),
	_cameraRotateSpeed(0.25f),
	_bMouseDown(false)
{

}

void SampleCommon::Startup()
{
	//Create Scene
	auto scene = std::make_shared<Scene>();
	Global::SetScene(scene);

	//Create View
	_renderView = std::make_shared<RenderView>();
	scene->AddView(_renderView);

	//Set Resize Events
	Global::GetRenderEngine()->GetWindow()->OnResizeEvent().connect(
		[&](const Ptr<Window> & window, int32_t preWidth, int32_t preHeight)
	{
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(Global::GetRenderEngine()->GetWindow()->Width());
		vp.height = static_cast<float>(Global::GetRenderEngine()->GetWindow()->Height());
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		_renderView->SetViewport(vp);
	});

	//Create Camera
	auto camera = std::make_shared<PhysicalCamera>(0.1f, 1e+3f);
	camera->SetPos(XMFLOAT3(0.0f, 3.0f, 0.0f));
	_renderView->SetCamera(camera);

	//Create Viewport
	RenderViewport vp;
	vp.topLeftX = 0.0f;
	vp.topLeftY = 0.0f;
	vp.width = static_cast<float>(Global::GetRenderEngine()->GetWindow()->Width());
	vp.height = static_cast<float>(Global::GetRenderEngine()->GetWindow()->Height());
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	_renderView->SetViewport(vp);

	//Set Background Render
	auto skyboxTex = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>()->AcquireResource(L"uffizi_cross.dds");
	auto skyboxRender = std::make_shared<SkyBox>();
	skyboxRender->SetTexture(skyboxTex);
	Global::GetRenderEngine()->GetRenderFramework()->GetSceneRenderer()->SetBackgroundRender(skyboxRender);

	//Init Inputs
	auto numMouses = Global::GetInputEngine()->NumInputDevices(INPUT_DEVICE_MOUSE);
	for (int32_t i = 0; i < numMouses; ++i)
	{
		auto mouse = std::static_pointer_cast<InputMouse>(Global::GetInputEngine()->GetInputDevice(INPUT_DEVICE_MOUSE, i));
		mouse->OnButtonDownEvent().connect(std::bind(&SampleCommon::MouseDown, this, std::placeholders::_1, std::placeholders::_2));
		mouse->OnButtonUpEvent().connect(std::bind(&SampleCommon::MouseUp, this, std::placeholders::_1, std::placeholders::_2));
		mouse->OnMouseMoveEvent().connect(std::bind(&SampleCommon::MouseMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
		float angleX = _cameraRotateSpeed * XMConvertToRadians((float)(relativeX));
		float angleY = _cameraRotateSpeed * XMConvertToRadians((float)(relativeY));

		_renderView->GetCamera()->Rotate(XMFLOAT3(0.0f, 1.0f, 0.0f), angleX);
		_renderView->GetCamera()->Pitch(angleY);
	}
}

bool SampleCommon::IsPosInWindow(const ToyGE::int2 & pos) const
{
	return pos.x >= 0 && pos.x <= Global::GetRenderEngine()->GetWindow()->Width()
		&& pos.y >= 0 && pos.y <= Global::GetRenderEngine()->GetWindow()->Height();
}