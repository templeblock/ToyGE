#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\Actor.h"
#include "ToyGE\RenderEngine\SceneCuller.h"
#include "ToyGE\RenderEngine\Effects\AmbientMap.h"

namespace ToyGE
{
	class RenderView;
	class RenderEffect;

	class TOYGE_CORE_API Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Scene();

		~Scene();

		void Tick(float elapsedTime);

		int32_t AddActor(const Ptr<Actor> & actor);

		Ptr<Actor> GetActor(int32_t actorID) const;

		void RemoveActor(const Ptr<Actor> & actor);

		void RemoveActor(int32_t objID);

		void AddView(const Ptr<RenderView> & view);

		int32_t NumViews() const
		{
			return static_cast<int32_t>(_views.size());
		}

		Ptr<RenderView> GetView(int32_t index) const
		{
			return _views[index];
		}

		CLASS_SET(AmbientColor, float3, _ambientColor);
		CLASS_GET(AmbientColor, float3, _ambientColor);

		void SetAmbientMapTexture(const Ptr<class Texture> & ambientMapTex, AmbientMapType type);
		CLASS_GET(AmbientMap, Ptr<class AmbientMap>, _ambientMap);

		void UpdateAmbientReflectionMap(const Ptr<Texture> & reflectionTex);
		CLASS_GET(AmbientReflectionMap, Ptr<class ReflectionMap>, _ambientReflectionMap);

		void SetRenderSun(bool bRenderSun)
		{
			_bRenderSun = bRenderSun;
		}
		bool IsRenderSun() const
		{
			return _bRenderSun;
		}

		void AddReflectionMapCapture(const Ptr<class ReflectionMapCapture> & capture)
		{
			_reflectionMapCaptures.push_back(capture);
		}
		const std::vector<Ptr<class ReflectionMapCapture>> & GetReflectionMapCaptures() const
		{
			return _reflectionMapCaptures;
		}

		void InitReflectionMaps();

		Ptr<Texture> GetReflectionMaps() const
		{
			return _reflectionMaps;
		}
		Ptr<class RenderBuffer> GetCapturesPosRadiusBuffer() const
		{
			return _capturesPosRadiusBuffer;
		}

	private:
		static int32_t actorID;
		std::map<int32_t, Ptr<Actor>> _actorsMap;

		std::vector<Ptr<RenderView>> _views;

		std::vector<Ptr<class ReflectionMapCapture>> _reflectionMapCaptures;
		Ptr<Texture> _reflectionMaps;
		Ptr<class RenderBuffer> _capturesPosRadiusBuffer;

		float3 _ambientColor;
		Ptr<class AmbientMap> _ambientMap;
		Ptr<class ReflectionMap> _ambientReflectionMap;
		bool _bRenderSun = false;
	};
}

#endif