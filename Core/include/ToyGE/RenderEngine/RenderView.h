#pragma once
#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\SceneRenderer.h"

namespace ToyGE
{
	class RenderComponent;
	class LightComponent;
	class Camera;
	class Scene;
	class RenderBuffer;
	class PostProcessing;
	class Texture;
	class Shader;
	class Material;

	struct ViewParams
	{
		float4x4	worldToViewMatrix;
		float4x4	viewToClipMatrix;
		float4x4	viewToClipMatrixNotJitter;
		float4x4	worldToClipMatrix;
		float4x4	worldToClipMatrixNoJitter;
		float4x4	viewToWorldMatrix;
		float4x4	clipToViewMatrix;
		float4x4	clipToViewMatrixNoJitter;
		float4x4	clipToWorldMatrix;
		float4x4	clipToWorldMatrixNoJitter;
		float4x4	preWorldToViewMatrix;
		float4x4	preViewToClipMatrix;
		float4x4	preWorldToClipMatrix;
		float4x4	clipToPreClipMatrix;
		float3		viewPos;
		float		_pad_0;
		float		viewNear;
		float		viewFar;
		float		viewLength;
		float		_pad_1;
		float2		viewSize;
		float2		invViewSize;
		float3		viewWorldDir;
		float		_pad_2;
	};

	class MatCmp
	{
	public:
		uint32_t GetMatKey(const Ptr<Material> & mat) const;

		bool operator()(const Ptr<Material> & mat0, const Ptr<Material> & mat1) const;
	};

	class TOYGE_CORE_API DrawingPolicy
	{
	public:
		virtual ~DrawingPolicy() = default;

		virtual void BindView(const Ptr<class RenderView> & view) {};

		virtual void BindMaterial(const Ptr<class Material> & mat) {};

		//virtual void BindTransform(const Ptr<class TransformComponent> & transform) {};

		virtual void Draw(const Ptr<class RenderComponent> & renderComponent) {};
	};

	class TOYGE_CORE_API PrimitiveDrawList
	{
	public:
		using PrimitiveDrawBatch = std::vector< Ptr<class RenderComponent> >;

		std::map<Ptr<Material>, PrimitiveDrawBatch, MatCmp> drawBatches;

		void AddRenderComponent(const Ptr<RenderComponent> & component);

		void Draw(const Ptr<DrawingPolicy> & drawingPolicy)
		{
			for (auto & drawBatchPair : drawBatches)
			{
				auto & mat = drawBatchPair.first;
				drawingPolicy->BindMaterial(mat);

				for (auto & drawComponent : drawBatchPair.second)
				{
					drawingPolicy->Draw(drawComponent);
				}
			}
		}

		void Draw(const Ptr<DrawingPolicy> & drawingPolicy, const Ptr<class RenderView> & view)
		{
			drawingPolicy->BindView(view);

			Draw(drawingPolicy);
		}
	};

	struct TOYGE_CORE_API ViewRenderContext
	{
		Ptr<PrimitiveDrawList> primitiveDrawList;
		std::vector<Ptr<LightComponent>> lights;
		std::map<String, PooledRenderResourceReference<RenderResource>> sharedResources;

		void SetSharedResource(const String & name, const PooledRenderResourceReference<RenderResource> & resource)
		{
			sharedResources[name] = resource;
		}

		template<class ResourceType>
		Ptr<ResourceType> GetSharedResource(const String & name) const
		{
			auto resFind = sharedResources.find(name);
			if (resFind != sharedResources.end())
				return resFind->second->Get()->Cast<ResourceType>();
			else
				return nullptr;
		}

		Ptr<Texture> GetSharedTexture(const String & name) const
		{
			return GetSharedResource<Texture>(name);
		}

		Ptr<RenderBuffer> GetBuffer(const String & name) const
		{
			return GetSharedResource<RenderBuffer>(name);
		}
	};

	class TOYGE_CORE_API RenderView : public std::enable_shared_from_this<RenderView>
	{
	public:
		SceneRenderingConfig	sceneRenderingConfig;
		PooledTextureRef		preAdaptedExposureScale;
		PooledTextureRef		preFrameResult;

		static const int32_t temporalAANumSamples = 8;
		float2 temporalAAJitter;

		RenderView();

		void PreRender();

		void PostRender();

		void UpdateParamsBuffer();

		void BindShaderParams(const Ptr<Shader> & shader) const;

		void SetScene(const Ptr<Scene> & scene)
		{
			_scene = scene;
		}
		Ptr<Scene> GetScene() const
		{
			return _scene.lock();
		}

		CLASS_SET(Viewport, RenderViewport, _viewport);
		CLASS_GET(Viewport, RenderViewport, _viewport);

		CLASS_SET(RenderTarget, Ptr<RenderTargetView>, _renderTarget);
		CLASS_GET(RenderTarget, Ptr<RenderTargetView>, _renderTarget);

		CLASS_SET(Camera, Ptr<Camera>, _camera);
		CLASS_GET(Camera, Ptr<Camera>, _camera);

		CLASS_GET(ViewRenderContext, Ptr<ViewRenderContext>, _viewRenderContext);

		CLASS_SET(PostProcessing, Ptr<PostProcessing>, _postProcessing);
		CLASS_GET(PostProcessing, Ptr<PostProcessing>, _postProcessing);

		CLASS_GET(ViewParams, ViewParams, _viewParams);

	protected:
		std::weak_ptr<Scene>	_scene;
		RenderViewport			_viewport;
		Ptr<RenderTargetView>	_renderTarget;
		Ptr<Camera>				_camera;
		Ptr<ViewRenderContext>	_viewRenderContext;
		Ptr<PostProcessing>		_postProcessing;
		ViewParams				_viewParams;
		Ptr<RenderBuffer>		_paramsBuffer;
	};
}

#endif