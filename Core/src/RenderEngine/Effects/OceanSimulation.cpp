#include "ToyGE\RenderEngine\Effects\OceanSimulation.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
	const float OceanSimulation::_gravity = 981.0f;

	OceanSimulation::OceanSimulation(int mapSize, float patchSize, float2 windDir, float windSpeed, float waveAmplitude)
		: _mapSize(mapSize),
		_patchSize(patchSize),
		_windDir(windDir),
		_windSpeed(windSpeed),
		_waveAmplitude(waveAmplitude)
	{
		InitHeightMap();

		auto texDesc = _initalHeightTex->Desc();
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		_htTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		_dkTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		_oceanSimulationFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"OceanSimulation.xml");
	}

	void OceanSimulation::Update(float elapsedTime)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		RenderContextStateSave saveState;
		rc->SaveState(
			RENDER_CONTEXT_STATE_DEPTHSTENCIL
			| RENDER_CONTEXT_STATE_RENDERTARGETS
			| RENDER_CONTEXT_STATE_INPUT
			| RENDER_CONTEXT_STATE_VIEWPORT, saveState);

		rc->SetRenderInput(CommonInput::QuadInput());

		RenderViewport vp;
		vp.topLeftX = vp.topLeftY = 0.0f;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = vp.height = static_cast<float>(_mapSize);

		_oceanSimulationFX->VariableByName("elapsedTime")->AsScalar()->SetValue(&elapsedTime, sizeof(float));

		_oceanSimulationFX->VariableByName("h0Tex")->AsShaderResource()->SetValue(_initalHeightTex->CreateTextureView());
		_oceanSimulationFX->VariableByName("omegaTex")->AsShaderResource()->SetValue(_omegaTex->CreateTextureView());
		rc->SetRenderTargets({ _htTex->CreateTextureView(), _dkTex->CreateTextureView() }, 0);
		_oceanSimulationFX->TechniqueByName("UpdateSpectrum")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_oceanSimulationFX->TechniqueByName("UpdateSpectrum")->PassByIndex(0)->UnBind();



		rc->RestoreState(saveState);
	}

	static float Phillips(float2 k, float2 w, float windVelocity, float a, float gravity)
	{
		float k_p2 = k.x * k.x + k.y * k.y;
		float kw = k.x * w.x + k.y * w.y;
		float l = windVelocity * windVelocity / gravity;

		float phillip = a * exp(-1.0f / (k_p2 * l * l)) / (k_p2 * k_p2) * (kw * kw);

		float sl = l / 1000.0f;
		return phillip * exp(-k_p2 * sl * sl);
	}

	static float GaussRandom()
	{
		float u1 = rand() / (float)RAND_MAX;
		float u2 = rand() / (float)RAND_MAX;
		if (u1 < 1e-6f)
			u1 = 1e-6f;
		return sqrtf(-2 * logf(u1)) * cosf(XM_2PI * u2);
	}

	void OceanSimulation::InitHeightMap()
	{
		std::vector<float2> h0(_mapSize * _mapSize);
		std::vector<float> omega(_mapSize * _mapSize);
		float2 k;
		for (int i = 0; i < _mapSize; ++i)
		{
			k.y = (static_cast<float>(-_mapSize) * 0.5f + static_cast<float>(i)) / _patchSize * (XM_2PI);
			for (int j = 0; j < _mapSize; ++j)
			{
				k.x = (static_cast<float>(-_mapSize) * 0.5f + static_cast<float>(j)) / _patchSize * (XM_2PI);
				float phillip = (k.x == 0 && k.y == 0) ? 0.0f : sqrtf(Phillips(k, _windDir, _windSpeed, _waveAmplitude, _gravity));
				float h = phillip * GaussRandom() / sqrtf(2.0f);
				h0[i * _mapSize + j] = h;

				omega[i * _mapSize + j] = sqrtf(_gravity * sqrtf(k.x * k.x + k.y * k.y));
			}
		}

		TextureDesc texDesc;
		texDesc.width = texDesc.height = _mapSize;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.cpuAccess = 0;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		RenderDataDesc initDataDesc;
		initDataDesc.pData = &h0[0];
		initDataDesc.rowPitch = sizeof(h0[0]) * _mapSize;
		initDataDesc.slicePitch = initDataDesc.rowPitch * _mapSize;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		_initalHeightTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, { initDataDesc });

		initDataDesc.pData = &omega[0];
		initDataDesc.rowPitch = sizeof(omega[0]) * _mapSize;
		initDataDesc.slicePitch = initDataDesc.rowPitch * _mapSize;
		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		_omegaTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, { initDataDesc });
	}

	Ptr<Texture> OceanSimulation::IFFT(Ptr<Texture> frequencyTex)
	{
		return Ptr<Texture>();
	}
}