#include "ToyGE\Kernel\EngineDriver.h"
#include "ToyGE\Platform\PlatformFactory.h"
#include "ToyGE\Platform\Windows\WindowsPlatformFactory.h"
#include "ToyGE\RenderEngine\RenderEngineCreateDef.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\Kernel\App.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\RenderEngine\Font\Font.h"
#include "ToyGE\Platform\Looper.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\Model.h"
#include "ToyGE\RenderEngine\Effects\DebugInfoRender.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Input\InputEngine.h"
#include "ToyGE\Input\Windows\WindowsInputEngine.h"
#include "ToyGE\RenderEngine\OctreeCuller.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\Font\BitmapFontFactory.h"
#include "ToyGE\RenderEngine\Font\BitmapFont.h"
#include "ToyGE\RenderEngine\Font\SignedDistanceFieldFontFactory.h"

namespace ToyGE
{
	void EngineDriver::StartUp(const Config & config, const Ptr<App> & app)
	{
		Logger::Init();

		if (!app)
		{
			Logger::LogLine("error> app null");
			Logger::LogLine("fatal error> abort");
			abort();
		}

		//Create PlatformFactory
#ifdef PLATFORM_WINDOWS
		Global::SetPlatformFactory(std::make_shared<WindowsPlatformFactory>());
#endif
		//Create Window
		WindowCreateParams windowParams;
		windowParams.name = config.windowTitle;
		windowParams.width = config.windowWidth;
		windowParams.height = config.windowHeight;
		windowParams.x = config.windowX;
		windowParams.y = config.windowY;
		auto window = Global::GetPlatformFactory()->CreateRenderWindow(windowParams);
		window->Init();
		if (!window)
		{
			Logger::LogLine("error> can not create window");
			Logger::LogLine("fatal error> abort");
			exit(0);
		}

		auto looper = Global::GetPlatformFactory()->CreateLooper();
		Global::SetLooper(looper);

		//Create InputEngine
#ifdef PLATFORM_WINDOWS
		auto inputEngine = std::make_shared<WindowsInputEngine>();
		
#endif
		inputEngine->SetWindow(window);
		inputEngine->RefreshInputDevices();
		Global::SetInputEngine(inputEngine);

		//Create RenderEngine
		String reDllPath = "ToyGE_RenderEngine_D3D11.dll";
		auto reDll = Global::GetPlatformFactory()->AcquireDll(reDllPath);
		if (!reDll)
		{
			Logger::LogLine("error> can not load render engine ['%s']", reDllPath);
			Logger::LogLine("fatal error> abort");
			exit(0);
		}

		auto engineCreateFunc = reinterpret_cast<CreateRenderEngineFunc>( reDll->GetProcAddress(CREATE_RE_FUNC_NAME) );
		if (!engineCreateFunc)
		{
			Logger::LogLine("error> can not find render engine create function ['%s']", reDllPath);
			Logger::LogLine("fatal error> abort");
			exit(0);
		}

		RenderEngine *pRe = nullptr;
		engineCreateFunc(window, &pRe);
		if (!pRe)
		{
			Logger::LogLine("error> can not create render engine ['%s']", reDllPath);
			Logger::LogLine("fatal error> abort");
			exit(0);
		}
		Global::SetRenderEngine( std::shared_ptr<RenderEngine>(pRe) );
		Logger::LogLine("info> render engine startup");
		Global::GetRenderEngine()->Startup();

		//Init Font
		Logger::LogLine("info> init font");
		FreetypeFont::InitLibrary();
		Global::GetRenderEngine()->SetFontFactory(std::make_shared<SignedDistanceFieldFontFactory>());
		//Font::Init();

		//Init ResourceManagers
		std::map<uint32_t, Ptr<ResourceManagerBase>> defaultResMap =
		{
			{ RESOURCE_EFFECT, std::make_shared<EffectManager>() },
			{ RESOURCE_TEXTURE, std::make_shared<TextureManager>() },
			{ RESOURCE_MODEL, std::make_shared<ModelManager>() },
			{ RESOURCE_MESH, std::make_shared<MeshManager>() },
			{ RESOURCE_MATERIAL, std::make_shared<MaterialManager>() },
			{ RESOURCE_FONT, std::make_shared<FontManager>() }
		};

		for (auto & res : config.resourceMap)
		{
			if (res.first == "Effect")
			{
				defaultResMap[RESOURCE_EFFECT]->As<EffectManager>()->SetBasePath(res.second);
			}
			else if (res.first == "Texture")
			{
				defaultResMap[RESOURCE_TEXTURE]->As<TextureManager>()->SetBasePath(res.second);
			}
			else if (res.first == "Model")
			{
				defaultResMap[RESOURCE_MODEL]->As<ModelManager>()->SetBasePath(res.second);
			}
			else if (res.first == "Font")
			{
				defaultResMap[RESOURCE_FONT]->As<FontManager>()->SetBasePath(res.second);
			}
		}
		for (auto & defaultRes : defaultResMap)
		{
			Global::SetResourceManager(defaultRes.first, defaultRes.second);
		}

		//Set Cullers
		pRe->SetSceneRenderObjsCuller(std::make_shared<DefaultRenderObjectCuller>());
		pRe->SetSceneRenderLightsCuller(std::make_shared<DefaultRenderLightCuller>());

		//Set RenderFramework
		pRe->SetRenderFramework(std::make_shared<DeferredRenderFramework>());
		pRe->GetRenderFramework()->Init();

		//auto renderActionQueue = std::make_shared<RenderActionQueue>();
		//renderActionQueue->AddRenderAction(std::make_shared<ForwardRenderFrame>());
		//renderActionQueue->AddRenderAction(std::make_shared<DebugInfoRender>());
		//Global::GetRenderEngine()->SetRenderActionQueue(renderActionQueue);

		//App Init
		Global::SetApp(app);
		Logger::LogLine("info> app startup");

		app->Startup();
	}

	void EngineDriver::Run()
	{
		auto looper = Global::GetLooper();
		Logger::LogLine("info> enter loop");
		looper->EnterLoop();
		Logger::LogLine("info> exit loop");
		Logger::LogLine("info> clear");
		Global::Clear();
		FreetypeFont::ClearLibrary();
		Logger::LogLine("info> system exit");
	}
}