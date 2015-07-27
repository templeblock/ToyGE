//#include "ToyGE\RenderEngine\Effects\SSAO.h"
//#include "ToyGE\Kernel\Global.h"
//#include "ToyGE\Kernel\ResourceManager.h"
//#include "ToyGE\RenderEngine\RenderEffect.h"
//#include "ToyGE\RenderEngine\RenderEffectVariable.h"
//#include "ToyGE\RenderEngine\RenderTechnique.h"
//#include "ToyGE\RenderEngine\RenderPass.h"
//#include "ToyGE\RenderEngine\RenderFactory.h"
//#include "ToyGE\RenderEngine\RenderEngine.h"
//#include "ToyGE\RenderEngine\RenderInput.h"
//#include "ToyGE\RenderEngine\RenderContext.h"
//#include "ToyGE\Math\Math.h"
//#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
//#include "ToyGE\RenderEngine\Texture.h"
//#include "ToyGE\RenderEngine\RenderUtil.h"
//#include "ToyGE\RenderEngine\Blur.h"
//#include "ToyGE\RenderEngine\Camera.h"
//#include "ToyGE\Platform\Window.h"
//#include "ToyGE\Kernel\Timer.h"
//#include <random>
//
//namespace ToyGE
//{
//	SSAO::SSAO()
//		: _radius(1.0f),
//		_bias(0.01f),
//		_intensity(0.5f)
//	{
//		_ssaoFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource("SSAO.xml");
//		//InitOffsetMap();
//	}
//
//	Ptr<Texture> SSAO::GetAOTexture(const Ptr<Texture> & depthLinearTex, const Ptr<Texture> & gbuffer0, const Ptr<Camera> & camera)
//	{
//		//auto depthLinearTex = sharedEnviroment->ParamByName("depthLinearTex")->As<SharedParamTexture>()->Texture();
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//		auto preInput = rc->GetRenderInput();
//		auto preRts = rc->GetRenderTargets();
//		auto preDs = rc->GetDepthStencil();
//		auto preVp = rc->GetViewport();
//
//		rc->SetDepthStencil(ResourceView());
//		rc->SetRenderInput(CommonInput::QuadInput());
//
//		auto aoTex = ScalableAO(depthLinearTex, gbuffer0, camera);
//
//		rc->SetRenderInput(preInput);
//		rc->SetRenderTargets(preRts, 0);
//		rc->SetDepthStencil(preDs);
//		rc->SetViewport(preVp);
//
//		return aoTex;
//		//sharedEnviroment->SetParam("aoTex", std::make_shared<SharedParamTexture>(aoTex));
//
//		//auto backFrameBuffer = Global::GetRenderEngine()->DefualtRenderTarget();
//		//Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ backFrameBuffer->CreateTextureView() }, 0);
//		//DebugQuad(aoTex->CreateTextureView(), COLOR_WRITE_R);
//	}
//
//	void SSAO::InitOffsetMap()
//	{
//		int tableSize = 8;
//		float angleStep = XM_2PI / static_cast<float>(tableSize);
//		std::default_random_engine e;
//		std::uniform_real_distribution<float> u(0, 1);
//		std::vector<float2> table;
//		for (int i = 0; i < tableSize; ++i)
//		{
//			float r = u(e);
//			float angle = angleStep * i;
//			table.push_back(float2(r * cos(angle), r * sin(angle)));
//		}
//
//		std::sort(table.begin(), table.end(),
//			[](const float2 & e0, const float2 e1) -> bool
//		{
//			return e0.x * e0.x + e0.y * e0.y < e1.x * e1.x + e1.y * e1.y;
//		});
//
//		//TextureDesc texDesc;
//		//texDesc.width = tableSize;
//		//texDesc.height = texDesc.depth = 1;
//		//texDesc.arraySize = 1;
//		//texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE;
//		//texDesc.cpuAccess = 0;
//		//texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
//		//texDesc.mipLevels = 1;
//		//texDesc.sampleCount = 1;
//		//texDesc.sampleQuality = 0;
//		//texDesc.type = TEXTURE_1D;
//
//		//RenderDataDesc dataDesc;
//		//dataDesc.pData = &table[0];
//		//dataDesc.rowPitch = sizeof(table[0]) * tableSize;
//		//_offsetMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc, { dataDesc });
//		//_ssaoFX->VariableByName("offsetMap")->AsShaderResource()->SetValue(_offsetMap->CreateTextureView());
//
//		auto offsetMapVar = _ssaoFX->VariableByName("offsetMap")->AsScalar();
//		for (int i = 0; i < tableSize; ++i)
//		{
//			offsetMapVar->SetValue(&table[i], sizeof(float2), sizeof(float4) * i);
//		}
//	}
//
//	Ptr<Texture> SSAO::AlchemyAO(const Ptr<Texture> & gbuffer, const Ptr<Camera> & camera)
//	{
//		auto texDesc = gbuffer->Desc();
//		texDesc.format = RENDER_FORMAT_R32_FLOAT;
//		texDesc.mipLevels = 1;
//		//texDesc.width = texDesc.width / 2;
//		//texDesc.height = texDesc.height / 2;
//		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
//
//		auto aoTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//
//		RenderViewport vp;
//		vp.topLeftX = vp.topLeftY = 0.0f;
//		vp.minDepth = 0.0f;
//		vp.maxDepth = 1.0f;
//		vp.width = static_cast<float>(texDesc.width);
//		vp.height = static_cast<float>(texDesc.height);
//		rc->SetViewport(vp);
//
//		auto &view = camera->ViewMatrix();
//		auto &proj = camera->ProjMatrix();
//		auto projXM = XMLoadFloat4x4(&proj);
//		auto invProjXM = XMMatrixInverse(&XMMatrixDeterminant(projXM), projXM);
//		XMFLOAT4X4 invProj;
//		XMStoreFloat4x4(&invProj, invProjXM);
//		float2 camNearFar(camera->Near(), camera->Far());
//
//		_ssaoFX->VariableByName("view")->AsScalar()->SetValue(&view);
//		_ssaoFX->VariableByName("proj")->AsScalar()->SetValue(&proj);
//		_ssaoFX->VariableByName("invProj")->AsScalar()->SetValue(&invProj);
//		_ssaoFX->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar, sizeof(camNearFar));
//
//		_ssaoFX->VariableByName("gbuffer")->AsShaderResource()->SetValue(gbuffer->CreateTextureView());
//		rc->SetRenderTargets({ aoTex->CreateTextureView() }, 0);
//		_ssaoFX->TechniqueByName("AlchemyAO")->PassByIndex(0)->Bind();
//		rc->DrawIndexed();
//		_ssaoFX->TechniqueByName("AlchemyAO")->PassByIndex(0)->UnBind();
//
//		//aoTex->GenerateMips();
//
//		rc->SetRenderTargets({ ResourceView() }, 0);
//
//		/*Blur blur(6);
//		aoTex = blur.GaussBlur(aoTex, 2, 0, 0, 1);*/
//
//		return aoTex;
//	}
//
//	Ptr<Texture> SSAO::ScalableAO(const Ptr<Texture> & depthLinearTex, const Ptr<Texture> & gbuffer0, const Ptr<Camera> & camera)
//	{
//		auto zPassTex = depthLinearTex;
//
//		auto texDesc = depthLinearTex->Desc();
//		texDesc.mipLevels = 1;
//		texDesc.format = RENDER_FORMAT_R8_UNORM;
//		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
//		texDesc.width /= 2;
//		texDesc.height /= 2;
//		auto aoTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
//		auto blurTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
//
//		auto rc = Global::GetRenderEngine()->GetRenderContext();
//		RenderViewport vp;
//		vp.topLeftX = vp.topLeftY = 0.0f;
//		vp.minDepth = 0.0f;
//		vp.maxDepth = 1.0f;
//		vp.width = static_cast<float>(texDesc.width);
//		vp.height = static_cast<float>(texDesc.height);
//		rc->SetViewport(vp);
//
//		auto &view = camera->ViewMatrix();
//		auto &proj = camera->ProjMatrix();
//		/*auto projXM = XMLoadFloat4x4(&proj);
//		auto invProjXM = XMMatrixInverse(&XMMatrixDeterminant(projXM), projXM);
//		XMFLOAT4X4 invProj;
//		XMStoreFloat4x4(&invProj, invProjXM);*/
//		float2 camNearFar(camera->Near(), camera->Far());
//
//		zPassTex->GenerateMips();
//
//		_ssaoFX->VariableByName("view")->AsScalar()->SetValue(&view);
//		_ssaoFX->VariableByName("proj")->AsScalar()->SetValue(&proj);
//		_ssaoFX->VariableByName("camNearFar")->AsScalar()->SetValue(&camNearFar, sizeof(camNearFar));
//
//		float4 scalableParam = float4(_radius, Global::GetRenderEngine()->GetWindow()->Height() * 0.5f, _bias, _intensity);
//		_ssaoFX->VariableByName("scalableParam")->AsScalar()->SetValue(&scalableParam);
//
//		_ssaoFX->VariableByName("depthLinearTex")->AsShaderResource()->SetValue(zPassTex->CreateTextureView(0, 0, 0, 1));
//		_ssaoFX->VariableByName("gbuffer0")->AsShaderResource()->SetValue(gbuffer0->CreateTextureView());
//		rc->SetRenderTargets({ aoTex->CreateTextureView() }, 0);
//		_ssaoFX->TechniqueByName("ScalableAO_SamplePass")->PassByIndex(0)->Bind();
//		rc->DrawIndexed();
//		_ssaoFX->TechniqueByName("ScalableAO_SamplePass")->PassByIndex(0)->UnBind();
//
//		auto gaussTableVar = _ssaoFX->VariableByName("gaussTable")->AsScalar();
//		int blurRadius = 4;
//		for (int i = 0; i <= blurRadius; ++i)
//		{
//			gaussTableVar->SetValue(&Blur::GaussTable(blurRadius)[i], sizeof(float), sizeof(float) * i);
//		}
//
//		//for (int i = 0; i < 2; ++i)
//		//{
//			_ssaoFX->VariableByName("aoTex")->AsShaderResource()->SetValue(aoTex->CreateTextureView());
//			rc->SetRenderTargets({ blurTex->CreateTextureView() }, 0);
//			_ssaoFX->TechniqueByName("ScalableAO_BlurX")->PassByIndex(0)->Bind();
//			rc->DrawIndexed();
//			_ssaoFX->TechniqueByName("ScalableAO_BlurX")->PassByIndex(0)->UnBind();
//
//			_ssaoFX->VariableByName("aoTex")->AsShaderResource()->SetValue(blurTex->CreateTextureView());
//			rc->SetRenderTargets({ aoTex->CreateTextureView() }, 0);
//			_ssaoFX->TechniqueByName("ScalableAO_BlurY")->PassByIndex(0)->Bind();
//			rc->DrawIndexed();
//			_ssaoFX->TechniqueByName("ScalableAO_BlurY")->PassByIndex(0)->UnBind();
//		//}
//
//		return aoTex;
//	}
//}