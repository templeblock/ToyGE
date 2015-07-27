#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
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

	enum TransientBufferType
	{
		TRANSIENTBUFFER_TEXT_VERTEX,
		TRANSIENTBUFFER_TEXT_INDEX
	};

	class TOYGE_CORE_API RenderEngine
	{
	public:
		RenderEngine(const Ptr<Window> & window);

		virtual ~RenderEngine();

		void Startup();

		void RenderFrame();

		const Ptr<Window> & GetWindow()
		{
			return _window;
		}

		const Ptr<RenderContext> & GetRenderContext()
		{
			return _renderContext;
		}

		const Ptr<RenderFactory> & GetRenderFactory()
		{
			return _renderFactory;
		}

		CLASS_SET(FontFactory, Ptr<FontFactory>, _fontFactory);
		CLASS_GET(FontFactory, Ptr<FontFactory>, _fontFactory);

		void SetRenderFramework(const Ptr<RenderFramework> & framework)
		{
			_renderFramework = framework;
		}

		Ptr<RenderFramework> GetRenderFramework() const
		{
			return _renderFramework;
		}

		Ptr<TransientBuffer> GetTransientBuffer(TransientBufferType type)
		{
			return _transientBufferMap[type];
		}

		const RenderDeviceAdapter & GetSelectedDeviceAdapter() const
		{
			return _adapter;
		}

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

		void PresentToBackBuffer(const ResourceView & resource);

	protected:
		Ptr<Window> _window;
		Ptr<RenderContext> _renderContext;
		Ptr<RenderFactory> _renderFactory;
		Ptr<FontFactory> _fontFactory;
		Ptr<RenderFramework> _renderFramework;
		Ptr<Texture> _defaultRenderTarget;
		Ptr<Texture> _defaultDepthStencil;
		RenderDeviceAdapter _adapter;
		Ptr<SceneCuller> _sceneRenderObjsCuller;
		Ptr<SceneCuller> _sceneRenderLightsCuller;

		std::map<TransientBufferType, Ptr<TransientBuffer>> _transientBufferMap;

		virtual void SwapChain() const = 0;

		virtual void DoStartup() = 0;

	};
}

#endif