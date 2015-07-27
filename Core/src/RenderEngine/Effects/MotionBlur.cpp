#include "ToyGE\RenderEngine\Effects\MotionBlur.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\Kernel\GlobalInfo.h"

namespace ToyGE
{
	MotionBlur::MotionBlur()
		: _exposureTime(10.0f),
		_maxVelocityLength(20),
		_mbNumSamples(20)
	{
		std::vector<MacroDesc> macros;
		macros.push_back({ "MAX_VELOCTYLENGTH", std::to_string(_maxVelocityLength) });
		macros.push_back({ "MB_NUMSAMPLES", std::to_string(_mbNumSamples) });

		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"MotionBlur.xml");
		_fx->SetExtraMacros(macros);
	}

	void MotionBlur::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		auto rawVelocityTex = sharedEnviroment->ParamByName(CommonRenderShareName::Velocity())->As<SharedParam<Ptr<Texture>>>()->GetValue();
		if (!rawVelocityTex)
			return;

		auto velocityTex = InitVelocityMap(rawVelocityTex);

		auto tileMaxTex = TileMax(velocityTex);

		auto neighborMaxTex = NeighborMax(tileMaxTex);

		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);
		auto sceneTex = sharedEnviroment->GetView()->GetRenderResult();
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);
		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		Blur(sceneTex, linearDepthTex, velocityTex, neighborMaxTex, sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		sharedEnviroment->GetView()->FlipRenderTarget();

		velocityTex->Release();
		tileMaxTex->Release();
		neighborMaxTex->Release();
		//sceneTex->Release();
	}

	Ptr<Texture> MotionBlur::InitVelocityMap(const Ptr<Texture> & rawVelocityTex)
	{
		auto texDesc = rawVelocityTex->Desc();
		auto velocityMap = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		float exposureTime = 10.0f;
		float veloctyScale = exposureTime * Global::GetInfo()->GetFPS();
		_fx->VariableByName("velocityScale")->AsScalar()->SetValue(&veloctyScale);

		_fx->VariableByName("velocityTex")->AsShaderResource()->SetValue(rawVelocityTex->CreateTextureView());
		rc->SetRenderTargets({ velocityMap->CreateTextureView() }, 0);

		_fx->TechniqueByName("InitVelocityMap")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("InitVelocityMap")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return velocityMap;
	}

	Ptr<Texture> MotionBlur::TileMax(const Ptr<Texture> & velocityTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto preVP = rc->GetViewport();

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		int tileSize = 20;

		//TileMax X
		auto texDesc = velocityTex->Desc();
		texDesc.width = (texDesc.width + tileSize - 1) / tileSize;
		auto tileMapTexTmp = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		rc->SetViewport(vp);

		float2 texelSize = 1.0f / float2(static_cast<float>(velocityTex->Desc().width), static_cast<float>(velocityTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("velocityTex")->AsShaderResource()->SetValue(velocityTex->CreateTextureView());
		rc->SetRenderTargets({ tileMapTexTmp->CreateTextureView() }, 0);
		_fx->TechniqueByName("TileMaxX")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("TileMaxX")->PassByIndex(0)->UnBind();

		//TileMax Y
		texDesc.height = (texDesc.height + tileSize - 1) / tileSize;
		auto tileMapTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		texelSize = 1.0f / float2(static_cast<float>(tileMapTexTmp->Desc().width), static_cast<float>(tileMapTexTmp->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		_fx->VariableByName("velocityTex")->AsShaderResource()->SetValue(tileMapTexTmp->CreateTextureView());
		rc->SetRenderTargets({ tileMapTex->CreateTextureView() }, 0);
		_fx->TechniqueByName("TileMaxY")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("TileMaxY")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		tileMapTexTmp->Release();

		return tileMapTex;
	}

	Ptr<Texture> MotionBlur::NeighborMax(const Ptr<Texture> & tileMaxTex)
	{
		auto texDesc = tileMaxTex->Desc();
		auto neighborMaxTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		float2 texelSize = 1.0f / float2(static_cast<float>(tileMaxTex->Desc().width), static_cast<float>(tileMaxTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto preVP = rc->GetViewport();
		auto vp = preVP;
		vp.width = static_cast<float>(texDesc.width);
		vp.height = static_cast<float>(texDesc.height);
		rc->SetViewport(vp);

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		_fx->VariableByName("tileMaxTex")->AsShaderResource()->SetValue(tileMaxTex->CreateTextureView());
		rc->SetRenderTargets({ neighborMaxTex->CreateTextureView() }, 0);
		_fx->TechniqueByName("NeighborMax")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("NeighborMax")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);

		return neighborMaxTex;
	}

	void MotionBlur::Blur(
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & linearDepthTex,
		const Ptr<Texture> & velocityTex,
		const Ptr<Texture> & neighborMaxTex,
		const ResourceView & target)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		auto texDesc = sceneTex->Desc();
		texDesc.mipLevels = 1;
		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		float2 texelSize = 1.0f / float2(static_cast<float>(sceneTex->Desc().width), static_cast<float>(sceneTex->Desc().height));
		_fx->VariableByName("texelSize")->AsScalar()->SetValue(&texelSize);

		auto frameCount = Global::GetInfo()->GetFrameCount();
		_fx->VariableByName("frameCount")->AsScalar()->SetValue(&frameCount);

		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
		_fx->VariableByName("velocityTex")->AsShaderResource()->SetValue(velocityTex->CreateTextureView());
		_fx->VariableByName("neighborMaxTex")->AsShaderResource()->SetValue(neighborMaxTex->CreateTextureView());

		rc->SetRenderInput(CommonInput::QuadInput());
		rc->SetDepthStencil(ResourceView());

		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());
		rc->SetRenderTargets({ tmpTex->CreateTextureView() }, 0);

		_fx->TechniqueByName("MotionBlur")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("MotionBlur")->PassByIndex(0)->UnBind();


		++frameCount;
		_fx->VariableByName("frameCount")->AsScalar()->SetValue(&frameCount);

		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(tmpTex->CreateTextureView());
		rc->SetRenderTargets({ target }, 0);

		_fx->TechniqueByName("MotionBlur")->PassByIndex(0)->Bind();
		rc->DrawIndexed();
		_fx->TechniqueByName("MotionBlur")->PassByIndex(0)->UnBind();

		tmpTex->Release();
	}
}