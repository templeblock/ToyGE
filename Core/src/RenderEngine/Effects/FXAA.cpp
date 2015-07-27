#include "ToyGE\RenderEngine\Effects\FXAA.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	FXAA::FXAA()
		: _fxaaConsoleRcpFrameOpt_N(0.5f),
		_fxaaQualitySubpix(0.75f),
		_fxaaQualityEdgeThreshold(0.166f),
		_fxaaQualityEdgeThresholdMin(0.0833f),
		_fxaaConsoleEdgeSharpness(8.0f),
		_fxaaConsoleEdgeThreshold(0.125f),
		_fxaaConsoleEdgeThresholdMin(0.05f)
	{
		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"FXAA.xml");
	}

	void FXAA::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);

		auto targetTex = sharedEnviroment->GetView()->GetRenderTarget();

		auto fxaaInTex = SetupFXAAInTex(sharedEnviroment->GetView()->GetRenderResult());

		float targetWidth = static_cast<float>(targetTex->Desc().width);
		float targetHeight = static_cast<float>(targetTex->Desc().height);
		float2 fxaaQualityRcpFrame = float2(1.0f / targetWidth, 1.0f / targetHeight);
		float4 fxaaConsoleRcpFrameOpt = _fxaaConsoleRcpFrameOpt_N * float4(-fxaaQualityRcpFrame.x, -fxaaQualityRcpFrame.y, fxaaQualityRcpFrame.x, fxaaQualityRcpFrame.y);
		float4 fxaaConsoleRcpFrameOpt2 = 2.0f * float4(-fxaaQualityRcpFrame.x, -fxaaQualityRcpFrame.y, fxaaQualityRcpFrame.x, fxaaQualityRcpFrame.y);
		float4 fxaaConsole360RcpFrameOpt2 = 4.0f * float4(2.0f * fxaaQualityRcpFrame.x, 2.0f * fxaaQualityRcpFrame.y, -fxaaQualityRcpFrame.x, -fxaaQualityRcpFrame.y);

		_fx->VariableByName("fxaaQualityRcpFrame")->AsScalar()->SetValue(&fxaaQualityRcpFrame);
		_fx->VariableByName("fxaaConsoleRcpFrameOpt")->AsScalar()->SetValue(&fxaaConsoleRcpFrameOpt);
		_fx->VariableByName("fxaaConsoleRcpFrameOpt2")->AsScalar()->SetValue(&fxaaConsoleRcpFrameOpt2);
		_fx->VariableByName("fxaaConsole360RcpFrameOpt2")->AsScalar()->SetValue(&fxaaConsole360RcpFrameOpt2);

		_fx->VariableByName("fxaaQualitySubpix")->AsScalar()->SetValue(&_fxaaQualitySubpix);
		_fx->VariableByName("fxaaQualityEdgeThreshold")->AsScalar()->SetValue(&_fxaaQualityEdgeThreshold);
		_fx->VariableByName("fxaaQualityEdgeThresholdMin")->AsScalar()->SetValue(&_fxaaQualityEdgeThresholdMin);
		_fx->VariableByName("fxaaConsoleEdgeSharpness")->AsScalar()->SetValue(&_fxaaConsoleEdgeSharpness);
		_fx->VariableByName("fxaaConsoleEdgeThreshold")->AsScalar()->SetValue(&_fxaaConsoleEdgeThreshold);
		_fx->VariableByName("fxaaConsoleEdgeThresholdMin")->AsScalar()->SetValue(&_fxaaConsoleEdgeThresholdMin);

		_fx->VariableByName("fxaaInTex")->AsShaderResource()->SetValue(fxaaInTex->CreateTextureView());

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetRenderTargets({ targetTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("FXAA"), 0, 0, targetTex->Desc().width, targetTex->Desc().height);

		sharedEnviroment->GetView()->FlipRenderTarget();

		fxaaInTex->Release();
	}

	Ptr<Texture> FXAA::SetupFXAAInTex(const Ptr<Texture> & inTex)
	{
		auto resultTexDesc = inTex->Desc();
		resultTexDesc.format = RENDER_FORMAT_R8G8B8A8_UNORM;
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(resultTexDesc);

		_fx->VariableByName("fxaaInTex")->AsShaderResource()->SetValue(inTex->CreateTextureView());

		Global::GetRenderEngine()->GetRenderContext()->SetRenderTargets({ resultTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("FXAASetup"), 0, 0, inTex->Desc().width, inTex->Desc().height);

		return resultTex;
	}
}