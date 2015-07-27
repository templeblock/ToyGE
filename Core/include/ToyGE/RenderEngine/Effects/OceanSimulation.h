#pragma once
#ifndef OCEANSIMULATION_H
#define OCEANSIMULATION_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Texture;
	class RenderEffect;

	class TOYGE_CORE_API OceanSimulation
	{
	public:
		OceanSimulation(int mapSize, float patchSize, float2 windDir, float windSpeed, float waveAmplitude);

		void Update(float elapsedTime);

		Ptr<Texture> InitalHeightTex() const
		{
			return _initalHeightTex;
		}

		Ptr<Texture> OmegaTex() const
		{
			return _omegaTex;
		}

		Ptr<Texture> HtTex() const
		{
			return _htTex;
		}

	private:
		const static float _gravity;
		int _mapSize;
		float _patchSize;
		float2 _windDir;
		float _windSpeed;
		float _waveAmplitude;
		Ptr<Texture> _initalHeightTex;
		Ptr<Texture> _omegaTex;
		Ptr<Texture> _htTex;
		Ptr<Texture> _dkTex;
		Ptr<RenderEffect> _oceanSimulationFX;

		void InitHeightMap();
		Ptr<Texture> IFFT(Ptr<Texture> frequencyTex);
	};
}

#endif