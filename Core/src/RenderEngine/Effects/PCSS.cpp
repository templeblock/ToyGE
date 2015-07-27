#include "ToyGE\RenderEngine\Effects\PCSS.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\RenderComponent.h"
#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\RenderEngine\Effects\CascadedShadowDepthTechnique.h"
#include "ToyGE\RenderEngine\Blur.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include <random>

namespace ToyGE
{
	ShadowRenderTechniquePCSS_SAVSM::ShadowRenderTechniquePCSS_SAVSM()
		: _pcssLookupTableSize(36)
	{
		InitPCSSLookupTable();
	}

	void ShadowRenderTechniquePCSS_SAVSM::BindParams(const Ptr<RenderEffect> & fx, const Ptr<LightComponent> & light, const Ptr<Texture> & shadowMap)
	{
		if (light->Type() == LIGHT_DIRECTIONAL
			|| light->Type() == LIGHT_POINT)
			return;

		ShadowRenderTechniqueSAVSM::BindParams(fx, light, shadowMap);

		//BindShadowTex(shadowMap, fx);
		fx->VariableByName("pcssLookupTable")->AsShaderResource()->SetValue(_pcssLookupTable->CreateTextureView());
		float4 pcssConfig;
		pcssConfig.x = static_cast<float>(_pcssLookupTableSize);
		pcssConfig.y = 5.0f;
		fx->VariableByName("pcssConfig")->AsScalar()->SetValue(&pcssConfig, sizeof(pcssConfig));
	}

	void ShadowRenderTechniquePCSS_SAVSM::InitPCSSLookupTable()
	{
		float angleStep = XM_2PI / static_cast<float>(_pcssLookupTableSize - 1);
		std::default_random_engine e;
		std::uniform_real_distribution<float> u(0, 1);
		std::vector<float2> table;
		table.push_back(float2(0.0f, 0.0f));
		for (int i = 0; i < _pcssLookupTableSize - 1; ++i)
		{
			float r = u(e);
			float angle = angleStep * i;
			table.push_back(float2(r * cos(angle), r * sin(angle)));
		}

		std::sort(table.begin(), table.end(),
			[](const float2 & e0, const float2 & e1) -> bool
		{
			return e0.x * e0.x + e0.y * e0.y < e1.x * e1.x + e1.y * e1.y;
		});

		TextureDesc texDesc;
		texDesc.width = _pcssLookupTableSize;
		texDesc.height = texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_1D;

		RenderDataDesc dataDesc;
		dataDesc.pData = &table[0];
		dataDesc.rowPitch = sizeof(table[0]) * _pcssLookupTableSize;
		_pcssLookupTable = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, { dataDesc });
	}
}