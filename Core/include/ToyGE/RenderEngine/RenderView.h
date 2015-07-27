#pragma once
#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class RenderComponent;
	class LightComponent;
	class RenderConfig;
	class Camera;
	class Scene;
	class RenderBuffer;
	class RenderAction;
	class RenderSharedEnviroment;
	class Texture;
	class RenderEffect;

	struct ViewParams
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 proj;
		XMFLOAT4X4 viewProj;
		XMFLOAT4X4 invView;
		XMFLOAT4X4 preView;
		float3 cameraPos;
		float _pad_0;
		float2 cameraNearFar;
		float2 _pad_1;
		float4 viewSize;
	};

	class TOYGE_CORE_API RenderView : public std::enable_shared_from_this<RenderView>
	{
	public:
		RenderView();

		void SetScene(const Ptr<Scene> & scene)
		{
			_scene = scene;
		}

		Ptr<Scene> GetScene() const
		{
			return _scene.lock();
		}

		CLASS_GET(Viewport, RenderViewport, _viewport);
		//CLASS_SET(Viewport, RenderViewport, _viewport);

		void SetViewport(const RenderViewport & viewport)
		{
			_viewport = viewport;
			InitRenderTarget(static_cast<int32_t>(viewport.width), static_cast<int32_t>(viewport.height));
		}

		void InitRenderTarget(int32_t width, int32_t height);

		void FlipRenderTarget()
		{
			std::swap(_renderTarget, _renderResult);
		}

		CLASS_GET(RenderTarget, Ptr<Texture>, _renderTarget);

		CLASS_GET(RenderResult, Ptr<Texture>, _renderResult);
		//CLASS_SET(RenderTarget, ResourceView, _renderTarget);

		CLASS_GET(Camera, Ptr<Camera>, _camera);
		CLASS_SET(Camera, Ptr<Camera>, _camera);

		void InitForRender();

		const std::vector<Ptr<RenderComponent>> & GetRenderObjects() const
		{
			return _renderObjects;
		}

		const std::vector<Ptr<LightComponent>> & GetRenderLights() const
		{
			return _renderLights;
		}

		CLASS_GET(RenderConfig, Ptr<RenderConfig>, _renderConfig);
		CLASS_SET(RenderConfig, Ptr<RenderConfig>, _renderConfig);

		CLASS_GET(RenderSharedEnviroment, Ptr<RenderSharedEnviroment>, _renderSharedEnv);

		CLASS_GET(ViewParams, ViewParams, _viewParams);

		void AddPostProcessRender(const Ptr<RenderAction> & render)
		{
			_postProcessRenders.push_back(render);
		}

		void RemovePostProcessRender(const Ptr<RenderAction> & render)
		{
			auto itr = std::find(_postProcessRenders.begin(), _postProcessRenders.end(), render);
			if (itr != _postProcessRenders.end())
				_postProcessRenders.erase(itr);
		}

		void RenderPostProcess();

		void BindParams(const Ptr<RenderEffect> & effect) const;

	protected:
		std::weak_ptr<Scene> _scene;
		RenderViewport _viewport;
		//ResourceView _renderTarget;
		Ptr<Texture> _renderTarget;
		Ptr<Texture> _renderResult;
		Ptr<Camera> _camera;
		Ptr<RenderConfig> _renderConfig;
		//RenderActionQueuePtr _renderQueue;
		std::vector<Ptr<RenderComponent>> _renderObjects;
		std::vector<Ptr<LightComponent>> _renderLights;
		Ptr<RenderSharedEnviroment> _renderSharedEnv;
		std::vector<Ptr<RenderAction>> _postProcessRenders;
		Ptr<RenderBuffer> _paramsBuffer;
		ViewParams _viewParams;

		void UpdateParamsBuffer();
	};
}

#endif