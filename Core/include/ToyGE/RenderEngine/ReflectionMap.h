#pragma once
#ifndef REFLECTIONMAP_H
#define REFLECTIONMAP_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "boost\noncopyable.hpp"

namespace ToyGE
{
	class Texture;
	class RenderEffect;

	class TOYGE_CORE_API ReflectionMap : public std::enable_shared_from_this<ReflectionMap>, public boost::noncopyable
	{
	public:
		static Ptr<ReflectionMap> Create();

		static Ptr<ReflectionMap> GetReflectionMap(int32_t id);

		static void RemoveReflectionMap(int32_t id);

		static void Clear();

		void SetEnvMap(const Ptr<Texture> & envMap)
		{
			_envMap = envMap;
		}

		const Ptr<Texture> & GetEnvMap() const
		{
			return _envMap;
		}

		void InitPreComputedData();

		int32_t GetID() const
		{
			return _id;
		}

		void BindEffectParams(const Ptr<RenderEffect> & fx);

	private:
		static int32_t _defaultMapSize;
		static int32_t _idAll;
		static std::map<int32_t, Ptr<ReflectionMap>> _reflectionMaps;
		int32_t _id;
		int32_t _mapSize;
		Ptr<RenderEffect> _fx;
		Ptr<Texture> _envMap;
		Ptr<Texture> _prefiltedEnvMap;
		Ptr<Texture> _LUT;

		ReflectionMap();

		void InitMapTextures();
	};
}

#endif