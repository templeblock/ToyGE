#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Core.h"

namespace ToyGE
{
	Ptr<Config> Global::_config;
	Ptr<App> Global::_app;
	Ptr<Window> Global::_window;
	Ptr<Scene> Global::_scene;
	Ptr<Platform> Global::_platform;
	Ptr<RenderEngine> Global::_renderEngine;
	Ptr<FontFactory> Global::_fontFactory;
	Ptr<Font> Global::_font;
	Ptr<InputEngine> Global::_inputEngine;
	Ptr<Looper> Global::_looper;
	Ptr<GlobalInfo> Global::_info = std::make_shared<GlobalInfo>();
	std::map<TransientBufferType, Ptr<TransientBuffer>> Global::_transientBufferMap;

	void Global::Clear()
	{
		_config.reset();
		_app.reset();
		_window.reset();
		_platform.reset();
		_inputEngine.reset();
		_transientBufferMap.clear();
		_looper.reset();
		_scene.reset();
		_fontFactory.reset();
		_font.reset();
		_renderEngine.reset();
	}
}