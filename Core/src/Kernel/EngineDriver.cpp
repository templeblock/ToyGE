#include "ToyGE\Kernel\EngineDriver.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\App.h"
#include "ToyGE\Kernel\Assertion.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Asset.h"
#include "ToyGE\Kernel\Config.h"
#include "ToyGE\Platform\Platform.h"
#include "ToyGE\Platform\File.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\Platform\Looper.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Platform\Windows\WindowsPlatform.h"
#include "ToyGE\Input\InputEngine.h"
#include "ToyGE\Input\Windows\WindowsInputEngine.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\RenderEngine\Font\BitmapFontFactory.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFontFactory.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\TransientBuffer.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"

namespace ToyGE
{
	void EngineDriver::Init(const Ptr<App> & app)
	{
		// Platform
#ifdef TOYGE_PLATFORM_WINDOWS
		Global::SetPlatform(std::make_shared<WindowsPlatform>());
#endif
		Global::GetPlatform()->Init();

		// Logger
		Logger::Init();

		// Init working path
		auto workingDir = ParentPath( Global::GetPlatform()->GetCurentProgramPath() );
		while ( !Global::GetPlatform()->IsPathDirectory(workingDir + "/" + "Config") )
		{
			workingDir = ParentPath(workingDir);
			if (workingDir.back() == ':')
			{
				ToyGE_LOG(LT_ERROR, "Cannot find base working path!");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}
		}
		Global::GetPlatform()->SetWorkingDirectory(workingDir);

		Shader::SetShaderFilesBasePath("Shaders");
		Asset::SetAssetsBasePath("Assets");

		if (!app)
		{
			ToyGE_LOG(LT_ERROR, "App is null!");
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}

		// Load config
		auto config = Config::Load("Config/Config.xml");
		if (!config)
		{
			ToyGE_LOG(LT_ERROR, "Cannot open config file!");
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}
		Global::SetConfig(config);

		// Create Window
		WindowCreateParams windowParams;
		windowParams.name = config->windowTitle;
		windowParams.width = config->windowWidth;
		windowParams.height = config->windowHeight;
		windowParams.x = config->windowX;
		windowParams.y = config->windowY;
		auto window = Global::GetPlatform()->CreatePlatformWindow(windowParams);
		if (!window)
		{
			ToyGE_LOG(LT_ERROR, "Cannot open create window! width=%d height=%d x=%d y=%d", windowParams.width, windowParams.height, windowParams.x, windowParams.y);
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}
		window->Init();
		Global::SetWindow(window);

		// Create Looper
		auto looper = Global::GetPlatform()->CreateLooper();
		Global::SetLooper(looper);

		// Init input
#ifdef TOYGE_PLATFORM_WINDOWS
		auto inputEngine = std::make_shared<WindowsInputEngine>();
#endif
		inputEngine->SetWindow(window);
		inputEngine->RefreshInputDevices();
		Global::SetInputEngine(inputEngine);

		// Init render engine
#ifdef TOYGE_PLATFORM_WINDOWS
		String reDllName = "ToyGE_RenderEngine_D3D11.dll";
#endif
		ToyGE_LOG(LT_INFO, "Begin init render engine! module=%s", reDllName.c_str());
		auto reDll = Global::GetPlatform()->FindDll(reDllName);
		if (!reDll)
		{
			ToyGE_LOG(LT_ERROR, "Cannot load render engine! module=%s", reDllName.c_str());
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}

		auto reCreateFunc = reinterpret_cast<CreateRenderEngineFunc>( reDll->GetProcAddress("CreateRenderEngine") );
		if (!reCreateFunc)
		{
			ToyGE_LOG(LT_ERROR, "Cannot find render engine create function! module=%s", reDllName.c_str());
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}

		RenderEngine * pRe = nullptr;
		reCreateFunc(&pRe);
		if (!pRe)
		{
			ToyGE_LOG(LT_ERROR, "Cannot create render engine! module=%s", reDllName.c_str());
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}
		Global::SetRenderEngine( std::shared_ptr<RenderEngine>(pRe) );
		RenderEngineInitParams reInitParams;
		reInitParams.adapterIndex = config->adapterIndex;
		reInitParams.adapterSelectKey = config->adapterSelectKey;
		reInitParams.bGrapicsEngineDebug = config->bGraphicsEngineDebug;
		Global::GetRenderEngine()->Init(reInitParams);
		ToyGE_LOG(LT_INFO, "End init render engine! module=%s", reDllName.c_str());

		// Init transient buffer
		int32_t initNumTextChars = 1024;
		int32_t charVertexSize = static_cast<int32_t>(sizeof(float) * (3 + 3));
		auto tb = Global::GetRenderEngine()->GetRenderFactory()->CreateTransientBuffer();
		tb->Init(charVertexSize, initNumTextChars * 4, BUFFER_BIND_VERTEX);
		tb->Register();
		Global::SetTransientBuffer(TRANSIENTBUFFER_TEXT_VERTEX, tb);

		tb = Global::GetRenderEngine()->GetRenderFactory()->CreateTransientBuffer();
		tb->Init(sizeof(uint32_t), initNumTextChars * 6, BUFFER_BIND_INDEX);
		tb->Register();
		Global::SetTransientBuffer(TRANSIENTBUFFER_TEXT_INDEX, tb);

		// Init fonts
		FontAsset::InitLibrary();
		Global::SetFontFactory(std::make_shared<SignedDistanceFieldFontFactory>());
		Global::SetFont( Font::Find("Fonts/STZHONGS.TTF") );

		// Init relfection map brdf lut
		ReflectionMap::InitLUT();

		// App Init
		Global::SetApp(app);
		ToyGE_LOG(LT_INFO, "Begin app init");
		app->Init();
	}

	void EngineDriver::Run()
	{
		// Show window
		Global::GetWindow()->Show();

		// Begin loop
		auto looper = Global::GetLooper();
		ToyGE_LOG(LT_INFO, "Begin loop");
		looper->EnterLoop();
		ToyGE_LOG(LT_INFO, "End loop");

		// Save dirty meshes
		for (auto & assetPair : Asset::GetAssetsMap())
		{
			if (assetPair.second->IsDirty())
				assetPair.second->Save();
		}

		// Save shader cache
		for (auto & shaderMeta : ShaderMetaType::GetShaderMetaTypeList())
			shaderMeta->UpdateFileCache();

		// Clear
		for (auto & resource : RenderResource::GetRenderResourceList())
		{
			if(!resource.expired())
				resource.lock()->Release();
		}
		Global::Clear();

		FontAsset::ClearLibrary();

		// End
		ToyGE_LOG(LT_INFO, "System exit");
		Logger::Release();
	}
}