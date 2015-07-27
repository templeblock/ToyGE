#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\GlobalInfo.h"
#include "ToyGE\Kernel\App.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"

namespace ToyGE
{
	Ptr<App> Global::_app;

	Ptr<Scene> Global::_scene;

	Ptr<PlatformFactory> Global::_platformFactory;

	Ptr<RenderEngine> Global::_renderEngine;

	Ptr<InputEngine> Global::_inputEngine;

	std::map<uint32_t, Ptr<ResourceManagerBase>> Global::_resourceManagerMap;

	Ptr<Looper> Global::_looper;

	Ptr<GlobalInfo> Global::_info = std::make_shared<GlobalInfo>();

	std::shared_ptr<DebugReporter> Global::_dbgReporter;

	void Global::SetApp(const Ptr<App> & app)
	{
		ToyGE_ASSERT(app);
		_app = app;
	}

	const Ptr<App> & Global::GetApp()
	{
		return _app;
	}

	void Global::SetScene(const Ptr<Scene> & scene)
	{
		_scene = scene;
	}

	const Ptr<Scene> & Global::GetScene()
	{
		return _scene;
	}

	void Global::SetPlatformFactory(const Ptr<PlatformFactory> & factory)
	{
		ToyGE_ASSERT(factory);
		_platformFactory = factory;
	}

	const Ptr<PlatformFactory> & Global::GetPlatformFactory()
	{
		return _platformFactory;
	}

	void Global::SetRenderEngine(const Ptr<RenderEngine> & renderEngine)
	{
		ToyGE_ASSERT(renderEngine);
		_renderEngine = renderEngine;
	}

	const Ptr<RenderEngine> & Global::GetRenderEngine()
	{
		return _renderEngine;
	}

	void Global::SetInputEngine(const Ptr<InputEngine> & inputEngine)
	{
		_inputEngine = inputEngine;
	}

	const Ptr<InputEngine> & Global::GetInputEngine()
	{
		return _inputEngine;
	}

	void Global::SetResourceManager(uint32_t type, const Ptr<ResourceManagerBase> & resManager)
	{
		_resourceManagerMap[type] = resManager;
	}

	Ptr<ResourceManagerBase> Global::GetResourceManager(uint32_t type)
	{
		auto resManagerFind = _resourceManagerMap.find(type);
		if (resManagerFind != _resourceManagerMap.end())
			return resManagerFind->second;
		else
			return Ptr<ResourceManagerBase>();
	}

	void Global::SetLooper(const Ptr<Looper> & looper)
	{
		_looper = looper;
	}

	const Ptr<Looper> & Global::GetLooper()
	{
		return _looper;
	}

	void Global::SetDebugReporter(const std::shared_ptr<DebugReporter> & reporter)
	{
		_dbgReporter = reporter;
	}

	const Ptr<DebugReporter> & Global::GetDebugReporter()
	{
		return _dbgReporter;
	}

	const Ptr<GlobalInfo> & Global::GetInfo()
	{
		return _info;
	}

	void Global::Clear()
	{
		_app.reset();

		_inputEngine.reset();

		_platformFactory.reset();

		for (auto & resManager : _resourceManagerMap)
			resManager.second->Clear();
		_resourceManagerMap.clear();

		_scene.reset();

		_looper.reset();

		CommonInput::Clear();

		ReflectionMap::Clear();

		_renderEngine.reset();
	}
}