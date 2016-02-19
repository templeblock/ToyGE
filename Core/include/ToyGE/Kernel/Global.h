#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"

namespace ToyGE
{
	enum TransientBufferType
	{
		TRANSIENTBUFFER_TEXT_VERTEX,
		TRANSIENTBUFFER_TEXT_INDEX
	};

	class TOYGE_CORE_API Global
	{
	public:
		static void SetConfig(const Ptr<class Config> & config)
		{
			_config = config;
		}
		static const Ptr<class Config> & GetConfig()
		{
			return _config;
		}

		static void SetApp(const Ptr<class App> & app)
		{
			_app = app;
		}
		static const Ptr<class App> & GetApp()
		{
			return _app;
		}

		static void SetWindow(const Ptr<class Window> & window)
		{
			_window = window;
		}
		static const Ptr<class Window> & GetWindow()
		{
			return _window;
		}

		static void SetScene(const Ptr<class Scene> & scene)
		{
			_scene = scene;
		}
		static const Ptr<class Scene> & GetScene()
		{
			return _scene;
		}

		static void SetPlatform(const Ptr<class Platform> & platform)
		{
			_platform = platform;
		}
		static const Ptr<class Platform> & GetPlatform()
		{
			return _platform;
		}

		static void SetRenderEngine(const Ptr<class RenderEngine> & renderEngine)
		{
			_renderEngine = renderEngine;
		}
		static const Ptr<class RenderEngine> & GetRenderEngine()
		{
			return _renderEngine;
		}

		static void SetFontFactory(const Ptr<class FontFactory> & fontFactory)
		{
			_fontFactory = fontFactory;
		}
		static const Ptr<class FontFactory> & GetFontFactory()
		{
			return _fontFactory;
		}

		static void SetFont(const Ptr<class Font> & font)
		{
			_font = font;
		}
		static const Ptr<class Font> & GetFont()
		{
			return _font;
		}

		static void SetInputEngine(const Ptr<class InputEngine> & inputEngine)
		{
			_inputEngine = inputEngine;
		}
		static const Ptr<class InputEngine> & GetInputEngine()
		{
			return _inputEngine;
		}

		static void SetLooper(const Ptr<class Looper> & looper)
		{
			_looper = looper;
		}
		static const Ptr<class Looper> & GetLooper()
		{
			return _looper;
		}

		static const Ptr<class GlobalInfo> & GetInfo()
		{
			return _info;
		}

		static void SetTransientBuffer(TransientBufferType type, const Ptr<class TransientBuffer> & buffer)
		{
			_transientBufferMap[type] = buffer;
		}
		static Ptr<class TransientBuffer> GetTransientBuffer(TransientBufferType type)
		{
			return _transientBufferMap[type];
		}

		static void Clear();
	private:
		static Ptr<class Config> _config;
		static Ptr<class App> _app;
		static Ptr<class Window> _window;
		static Ptr<class Scene> _scene;
		static Ptr<class Platform> _platform;
		static Ptr<class RenderEngine> _renderEngine;
		static Ptr<class FontFactory> _fontFactory;
		static Ptr<class Font> _font;
		static Ptr<class InputEngine> _inputEngine;
		static Ptr<class Looper> _looper;
		static Ptr<class GlobalInfo> _info;
		static std::map<TransientBufferType, Ptr<class TransientBuffer>> _transientBufferMap;
	};
}

#endif