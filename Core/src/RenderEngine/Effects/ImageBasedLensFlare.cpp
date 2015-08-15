#include "ToyGE\RenderEngine\Effects\ImageBasedLensFlare.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
	ImageBasedLensFlare::ImageBasedLensFlare()
		: _brightPassThreshold(0.5f),
		_spriteThreshold(1.0f),
		_flareIntensity(6.0f)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"ImageBasedLensFlare.xml");

		_lensTex = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>()->AcquireResource(L"Bokeh_Hex.dds");
	}

	void ImageBasedLensFlare::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();
		auto brightPassTex = BrightPass(sceneTex);

		auto setupTex = Setup(brightPassTex);

		LensBlur(setupTex, sharedEnviroment->GetView()->GetRenderResult());

		brightPassTex->Release();
		setupTex->Release();
	}

	Ptr<Texture> ImageBasedLensFlare::BrightPass(const Ptr<Texture> & sceneTex)
	{
		//float threshold = 0.5f;
		auto brightPassTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(sceneTex->Desc());

		_fx->VariableByName("brightPassThreshold")->AsScalar()->SetValue(&_brightPassThreshold);

		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ brightPassTex->CreateTextureView() }, 0);
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTargets(0.0f);

		RenderQuad(_fx->TechniqueByName("BrightPass"));

		return brightPassTex;
	}

	Ptr<Texture> ImageBasedLensFlare::Setup(const Ptr<Texture> & brightPassTex)
	{
		auto setupTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(brightPassTex->Desc());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("brightPassTex")->AsShaderResource()->SetValue(brightPassTex->CreateTextureView());

		rc->SetRenderTargets({ setupTex->CreateTextureView() }, 0);
		rc->ClearRenderTargets(0.0f);

		int2 texSize = int2(setupTex->Desc().width / 2, setupTex->Desc().height / 2);
		int2 center = texSize;

		for (int i = 0; i < 3; ++i)
		{
			int2 xy = center - texSize / 2;

			RenderQuad(_fx->TechniqueByName("LensFlareSetup"),
				xy.x, xy.y, texSize.x, texSize.y,
				0.0f, 0.0f, 1.0f, 1.0f);

			texSize /= 2;
		}

		for (int i = 0; i < 5; ++i)
		{
			int2 xy = center - texSize / 2;

			RenderQuad(_fx->TechniqueByName("LensFlareSetup"),
				xy.x, xy.y, texSize.x, texSize.y,
				1.0f, 1.0f, -1.0f, -1.0f);

			texSize *= 2;
		}

		return setupTex;
	}

	void ImageBasedLensFlare::LensBlur(const Ptr<Texture> & setupTex, const Ptr<Texture> & target)
	{
		int32_t tileSize = 9;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		//Extract Sprite Points
		int32_t extractWidth = (setupTex->Desc().width + tileSize - 1) / tileSize;
		int32_t extractHeight = (setupTex->Desc().height + tileSize - 1) / tileSize;

		RenderBufferDesc spPointsBufDesc;
		spPointsBufDesc.bindFlag = BUFFER_BIND_SHADER_RESOURCE | BUFFER_BIND_UNORDERED_ACCESS | BUFFER_BIND_STRUCTURED;
		spPointsBufDesc.elementSize = sizeof(float2) + sizeof(float3);
		spPointsBufDesc.numElements = extractWidth * extractHeight;
		spPointsBufDesc.cpuAccess = 0;
		spPointsBufDesc.structedByteStride = spPointsBufDesc.elementSize;

		auto spPointsBuf = Global::GetRenderEngine()->GetRenderFactory()->GetBufferPooled(spPointsBufDesc);

		_fx->VariableByName("spriteThreshold")->AsScalar()->SetValue(&_spriteThreshold);

		_fx->VariableByName("setupTex")->AsShaderResource()->SetValue(setupTex->CreateTextureView());
		_fx->VariableByName("spPointsBuf")->AsUAV()->SetValue(
			spPointsBuf->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, spPointsBufDesc.numElements, BUFFER_UAV_APPEND) );

		rc->SetRenderTargets({}, 0);
		RenderQuad(_fx->TechniqueByName("ExtractSpritePoints"),
			0, 0, extractWidth, extractHeight);

		//Render Sprites
		RenderBufferDesc indirectArgsBufDesc;
		indirectArgsBufDesc.bindFlag = BUFFER_BIND_INDIRECT_ARGS;
		indirectArgsBufDesc.elementSize = 16;
		indirectArgsBufDesc.numElements = 1;
		indirectArgsBufDesc.cpuAccess = 0;
		indirectArgsBufDesc.structedByteStride = 0;

		uint32_t initData[] = { 0, 1, 0, 0 };

		auto indirectAgsBuffer = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(indirectArgsBufDesc, &initData[0]);

		spPointsBuf->CopyStructureCountTo(indirectAgsBuffer, 0, 0, spPointsBuf->Desc().numElements, RENDER_FORMAT_UNKNOWN, BUFFER_UAV_APPEND);

		_fx->VariableByName("texSize")->AsScalar()->SetValue(&target->GetTexSize());
		_fx->VariableByName("flareIntensity")->AsScalar()->SetValue(&_flareIntensity);

		_fx->VariableByName("spPointsRenderBuf")->AsShaderResource()->SetValue(
			spPointsBuf->CreateBufferView(RENDER_FORMAT_UNKNOWN, 0, spPointsBufDesc.numElements));
		_fx->VariableByName("lensTex")->AsShaderResource()->SetValue(_lensTex->CreateTextureView());

		auto ri = std::make_shared<RenderInput>();
		ri->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);
		rc->SetRenderInput(ri);

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(target->Desc().width);
		vp.height = static_cast<float>(target->Desc().height);
		rc->SetViewport(vp);

		//auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(target->Desc());
		rc->SetRenderTargets({ target->CreateTextureView() }, 0);
		//rc->ClearRenderTargets(0.0f);

		rc->SetDepthStencil(ResourceView());

		_fx->TechniqueByName("LensBlur")->PassByIndex(0)->Bind();
		rc->DrawInstancedIndirect(indirectAgsBuffer, 0);
		_fx->TechniqueByName("LensBlur")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		//tmpTex->Release();
	}
}