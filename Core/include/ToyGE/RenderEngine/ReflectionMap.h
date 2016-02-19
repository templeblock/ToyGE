#pragma once
#ifndef REFLECTIONMAP_H
#define REFLECTIONMAP_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\Math\Math.h"
#include "boost\noncopyable.hpp"

namespace ToyGE
{
	DECLARE_SHADER(, PrefilterEnvMapPS, SHADER_PS, "IBLPreCompute", "PrefilterEnvMapPS", SM_4);
	DECLARE_SHADER(, PreComputedLUTPS, SHADER_PS, "IBLPreCompute", "PreComputedLUTPS", SM_4);

	class Texture;

	class TOYGE_CORE_API ReflectionMap : public std::enable_shared_from_this<ReflectionMap>, public boost::noncopyable
	{
	public:
		int32_t maskID = -1;

		static void SetLUTSize(int32_t size)
		{
			_lutSize = size;
		}

		static int32_t GetLUTSize()
		{
			return _lutSize;
		}

		static void InitLUT();

		ReflectionMap();

		~ReflectionMap() = default;

		CLASS_SET(EnvironmentMap, Ptr<Texture>, _enviromentMap);
		CLASS_GET(EnvironmentMap, Ptr<Texture>, _enviromentMap);

		CLASS_SET(PreComputedMapSize, int32_t, _preComputedMapSize);
		CLASS_GET(PreComputedMapSize, int32_t, _preComputedMapSize);

		void InitPreComputedData();

		void BindShaderParams(const Ptr<Shader> & shader);

	private:
		static Ptr<Texture> _lut;
		static int32_t _lutSize;

		int32_t _preComputedMapSize;
		Ptr<Texture> _enviromentMap;
		PooledTextureRef _prefiltedEnviromentMapRef;
	};
}

#endif