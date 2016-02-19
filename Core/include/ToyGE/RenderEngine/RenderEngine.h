#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

#include "AntTweakBar.h"

namespace ToyGE
{
	class Window;
	class RenderContext;
	class RenderFactory;
	class FontFactory;
	class RenderFramework;
	class TransientBuffer;
	class SceneCuller;
	class Texture;
	class SceneRenderer;

	struct RenderEngineInitParams
	{
		int32_t adapterIndex = -1;
		String adapterSelectKey;
		bool bGrapicsEngineDebug = false;
	};

	class TOYGE_CORE_API RenderEngine
	{
	public:
		virtual ~RenderEngine();

		virtual void Init(const RenderEngineInitParams & initParams);

		virtual void SwapChain() const = 0;

		void Render();

		CLASS_GET(RenderContext, Ptr<RenderContext>, _renderContext);

		CLASS_GET(RenderFactory, Ptr<RenderFactory>, _renderFactory);

		CLASS_SET(SceneRenderer, Ptr<SceneRenderer>, _sceneRender);
		CLASS_GET(SceneRenderer, Ptr<SceneRenderer>, _sceneRender);

		CLASS_GET(FrameBuffer, Ptr<Texture>, _frameBuffer);

		CLASS_GET(BackBuffer, Ptr<Texture>, _backBuffer);

		//Ptr<TransientBuffer> GetTransientBuffer(TransientBufferType type)
		//{
		//	return _transientBufferMap[type];
		//}

		const RenderDeviceAdapter & GetSelectedDeviceAdapter() const
		{
			return _adapter;
		}

		CLASS_GET(DeviceOutputMode, std::vector<std::vector<RenderDeviceOutputMode>>, _outputModesList);

		void SetSceneRenderObjsCuller(const Ptr<SceneCuller> & culler)
		{
			_sceneRenderObjsCuller = culler;
		}

		Ptr<SceneCuller> GetSceneRenderObjsCuller() const
		{
			return _sceneRenderObjsCuller;
		}

		void SetSceneRenderLightsCuller(const Ptr<SceneCuller> & culler)
		{
			_sceneRenderLightsCuller = culler;
		}

		Ptr<SceneCuller> GetSceneRenderLightsCuller() const
		{
			return _sceneRenderLightsCuller;
		}

		static ShaderModel GetShaderModel()
		{
			return _shaderModel;
		}

		CLASS_SET(Gamma, float, _gamma);
		CLASS_GET(Gamma, float, _gamma);

		bool IsFullScreen() const
		{
			return _bFullScreen;
		}
		virtual void SetFullScreen(bool bFullScreen);

	protected:
		Ptr<RenderContext>	_renderContext;
		Ptr<RenderFactory>	_renderFactory;
		Ptr<SceneRenderer>	_sceneRender;
		Ptr<Texture>		_frameBuffer;
		Ptr<Texture>		_backBuffer;
		RenderDeviceAdapter _adapter;
		std::vector<std::vector<RenderDeviceOutputMode>> _outputModesList;
		Ptr<SceneCuller>	_sceneRenderObjsCuller;
		Ptr<SceneCuller>	_sceneRenderLightsCuller;
		float _gamma = 2.2f;
		bool _bFullScreen = false;

		static ShaderModel _shaderModel;
	};

	using CreateRenderEngineFunc = void(*)(RenderEngine **ppRenderEngine);
}

#endif