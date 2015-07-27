#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class TOYGE_CORE_API DebugReporter
	{
	public:
		virtual void Report() = 0;
	};

	class App;
	class Scene;
	class PlatformFactory;
	class RenderEngine;
	class InputEngine;
	class ResourceManagerBase;
	class Looper;
	class GlobalInfo;

	class TOYGE_CORE_API Global
	{
	public:
		static void SetApp(const Ptr<App> & app);
		static const Ptr<App> & GetApp();

		static void SetScene(const Ptr<Scene> & scene);
		static const Ptr<Scene> & GetScene();

		static void SetPlatformFactory(const Ptr<PlatformFactory> & factory);
		static const Ptr<PlatformFactory> & GetPlatformFactory();

		static void SetRenderEngine(const Ptr<RenderEngine> & renderEngine);
		static const Ptr<RenderEngine> & GetRenderEngine();

		static void SetInputEngine(const Ptr<InputEngine> & inputEngine);
		static const Ptr<InputEngine> & GetInputEngine();

		static void SetResourceManager(uint32_t type, const Ptr<ResourceManagerBase> & resManager);
		static Ptr<ResourceManagerBase> GetResourceManager(uint32_t type);

		static void SetLooper(const Ptr<Looper> & looper);
		static const Ptr<Looper> & GetLooper();

		static void SetDebugReporter(const Ptr<DebugReporter> & reporter);
		static const Ptr<DebugReporter> & GetDebugReporter();

		static const Ptr<GlobalInfo> & GetInfo();

		static void Clear();
	private:
		static Ptr<App> _app;
		static Ptr<Scene> _scene;
		static Ptr<PlatformFactory> _platformFactory;
		static Ptr<RenderEngine> _renderEngine;
		static Ptr<InputEngine> _inputEngine;
		static std::map<uint32_t, Ptr<ResourceManagerBase>> _resourceManagerMap;
		static Ptr<Looper> _looper;
		static Ptr<GlobalInfo> _info;
		static Ptr<DebugReporter> _dbgReporter;
	};
}

#endif