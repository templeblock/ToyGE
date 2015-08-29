//#include "ToyGE\RenderEngine\Effects\DepthOfField2.h"
//#include "ToyGE\Kernel\Global.h"
//#include "ToyGE\Kernel\ResourceManager.h"
//#include "ToyGE\RenderEngine\RenderEffect.h"
//#include "ToyGE\RenderEngine\RenderEngine.h"
//#include "ToyGE\RenderEngine\RenderContext.h"
//#include "ToyGE\RenderEngine\RenderFactory.h"
//#include "ToyGE\RenderEngine\Texture.h"
//#include "ToyGE\RenderEngine\RenderUtil.h"
//#include "ToyGE\RenderEngine\Camera.h"
//#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
//#include "ToyGE\RenderEngine\RenderView.h"
//
//namespace ToyGE
//{
//	DepthOfField2::DepthOfField2()
//	{
//		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"DepthOfField2.xml");
//	}
//
//	void DepthOfField2::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
//	{
//		auto linearDepthTex = sharedEnviroment->GetTextureParam(CommonRenderShareName::LinearDepth());
//		auto rawDepthTex = sharedEnviroment->GetTextureParam(CommonRenderShareName::RawDepth());
//
//		auto camera = sharedEnviroment->GetView()->GetCamera();
//
//		auto cocTex = ComputeCoC(linearDepthTex, rawDepthTex, camera);
//
//		auto tileMaxTex = ComputeTileMax(cocTex, linearDepthTex);
//
//		auto alphaTex = ComputeAlpha(cocTex, tileMaxTex, linearDepthTex);
//
//		Blur(
//			sharedEnviroment->GetView()->GetRenderResult(),
//			tileMaxTex,
//			alphaTex,
//			sharedEnviroment->GetView()->GetRenderTarget());
//
//		sharedEnviroment->GetView()->FlipRenderTarget();
//
//		cocTex->Release();
//		tileMaxTex->Release();
//		alphaTex->Release();
//	}
//
//	Ptr<Texture> DepthOfField2::ComputeCoC(const Ptr<Texture> & linearDepthTex, const Ptr<Texture> & rawDepth, const Ptr<Camera> & camera)
//	{
//		auto texDesc = linearDepthTex->Desc();
//		texDesc.bindFlag = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE;
//		texDesc.format = RENDER_FORMAT_R16_FLOAT;
//		auto cocTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//		rc->ClearRenderTargets({ cocTex->CreateTextureView() }, 0.0f);
//
//		auto phyCam = std::static_pointer_cast<PhysicalCamera>(camera);
//
//		_fx->VariableByName("focalLength")->AsScalar()->SetValue(&phyCam->GetFocalLength());
//		_fx->VariableByName("focalDistance")->AsScalar()->SetValue(&phyCam->GetFocalDistance());
//		float apertureSize = phyCam->GetFocalLength() / phyCam->GetFStops() * 0.5f;
//		_fx->VariableByName("aperture")->AsScalar()->SetValue(&apertureSize);
//		float pixelsPerMM = static_cast<float>(texDesc.height) / phyCam->GetFilmSize().y;
//		_fx->VariableByName("pixelsPerMM")->AsScalar()->SetValue(&pixelsPerMM);
//		float2 camNearFar = float2(camera->Near(), camera->Far());
//		_fx->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar);
//
//		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
//
//		rc->SetRenderTargets({ cocTex->CreateTextureView() }, 0);
//
//		RenderQuad(_fx->TechniqueByName("ComputeCoC"),
//			0, 0, 0, 0,
//			0.0f, 0.0f, 1.0f, 1.0f,
//			rawDepth->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
//
//		return cocTex;
//	}
//
//	Ptr<Texture> DepthOfField2::ComputeTileMax(const Ptr<Texture> & cocTex, const Ptr<Texture> & linearDepthTex)
//	{
//		static const int32_t tileSize = 20;
//		int32_t tileTexWidth = (cocTex->Desc().width + tileSize - 1) / tileSize;
//		int32_t tileTexHeight = (cocTex->Desc().height + tileSize - 1) / tileSize;
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//
//		//TileX
//		auto texDesc = cocTex->Desc();
//		texDesc.width = tileTexWidth;
//		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
//		auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		_fx->SetExtraMacros({ {"TILE_INIT", ""} });
//
//		_fx->VariableByName("cocTex")->AsShaderResource()->SetValue(cocTex->CreateTextureView());
//		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
//
//		rc->SetRenderTargets({ tmpTex->CreateTextureView() }, 0);
//		RenderQuad(_fx->TechniqueByName("SetupTileX"));
//
//		//TileY
//		texDesc.height = tileTexHeight;
//		auto tileMaxTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		_fx->SetExtraMacros({});
//
//		_fx->VariableByName("tileMaxTex")->AsShaderResource()->SetValue(tmpTex->CreateTextureView());
//
//		rc->SetRenderTargets({ tileMaxTex->CreateTextureView() }, 0);
//		RenderQuad(_fx->TechniqueByName("SetupTileY"));
//
//		//NeighborMax
//		auto neighborMaxTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		_fx->VariableByName("texSize")->AsScalar()->SetValue(&neighborMaxTex->GetTexSize());
//
//		_fx->VariableByName("tileMaxTex")->AsShaderResource()->SetValue(tileMaxTex->CreateTextureView());
//
//		rc->SetRenderTargets({ neighborMaxTex->CreateTextureView() }, 0);
//		RenderQuad(_fx->TechniqueByName("NeighborTile"));
//
//		tmpTex->Release();
//		tileMaxTex->Release();
//
//		return neighborMaxTex;
//	}
//
//	Ptr<Texture> DepthOfField2::ComputeAlpha(
//		const Ptr<Texture> & cocTex,
//		const Ptr<Texture> & tileMaxTex,
//		const Ptr<Texture> & linearDepthTex)
//	{
//		auto texDesc = cocTex->Desc();
//		texDesc.format = RENDER_FORMAT_R8G8_UNORM;
//		auto alphaTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		_fx->VariableByName("cocTex")->AsShaderResource()->SetValue(cocTex->CreateTextureView());
//		_fx->VariableByName("tileMaxTex")->AsShaderResource()->SetValue(tileMaxTex->CreateTextureView());
//		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());
//
//		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ alphaTex->CreateTextureView() }, 0);
//
//		RenderQuad(_fx->TechniqueByName("ComputeAlpha"));
//
//		return alphaTex;
//	}
//
//	void DepthOfField2::Blur(
//		const Ptr<Texture> & sceneTex,
//		const Ptr<Texture> & tileMaxTex,
//		const Ptr<Texture> & alphaTex,
//		const Ptr<Texture> & targetTex)
//	{
//		auto texDesc = sceneTex->Desc();
//		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
//
//		auto foregroundTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//		auto backgroundTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
//
//		_fx->VariableByName("texSize")->AsScalar()->SetValue(&sceneTex->GetTexSize());
//
//		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());
//		_fx->VariableByName("tileMaxTex")->AsShaderResource()->SetValue(tileMaxTex->CreateTextureView());
//		_fx->VariableByName("alphaTex")->AsShaderResource()->SetValue(alphaTex->CreateTextureView());
//
//		//Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets(
//		//{ foregroundTex->CreateTextureView(), backgroundTex->CreateTextureView() }, 0);
//
//		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ targetTex->CreateTextureView() }, 0);
//
//		RenderQuad(_fx->TechniqueByName("Blur"));
//	}
//}