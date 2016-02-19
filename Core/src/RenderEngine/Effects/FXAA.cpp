#include "ToyGE\RenderEngine\Effects\FXAA.h"
#include "ToyGE\Kernel\Core.h"

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
	}

	void FXAA::Render(const Ptr<RenderView> & view)
	{
		auto targetTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		auto fxaaInTexRef = SetupFXAAInTex(targetTex);

		float targetWidth = static_cast<float>(targetTex->GetDesc().width);
		float targetHeight = static_cast<float>(targetTex->GetDesc().height);
		float2 fxaaQualityRcpFrame = float2(1.0f / targetWidth, 1.0f / targetHeight);
		float4 fxaaConsoleRcpFrameOpt = _fxaaConsoleRcpFrameOpt_N * float4(-fxaaQualityRcpFrame.x(), -fxaaQualityRcpFrame.y(), fxaaQualityRcpFrame.x(), fxaaQualityRcpFrame.y());
		float4 fxaaConsoleRcpFrameOpt2 = 2.0f * float4(-fxaaQualityRcpFrame.x(), -fxaaQualityRcpFrame.y(), fxaaQualityRcpFrame.x(), fxaaQualityRcpFrame.y());
		float4 fxaaConsole360RcpFrameOpt2 = 4.0f * float4(2.0f * fxaaQualityRcpFrame.x(), 2.0f * fxaaQualityRcpFrame.y(), -fxaaQualityRcpFrame.x(), -fxaaQualityRcpFrame.y());

		auto ps = Shader::FindOrCreate<FXAAPS>();

		ps->SetScalar("fxaaQualityRcpFrame", fxaaQualityRcpFrame);
		ps->SetScalar("fxaaConsoleRcpFrameOpt", fxaaConsoleRcpFrameOpt);
		ps->SetScalar("fxaaConsoleRcpFrameOpt2", fxaaConsoleRcpFrameOpt2);
		ps->SetScalar("fxaaConsole360RcpFrameOpt2", fxaaConsole360RcpFrameOpt2);

		ps->SetScalar("fxaaQualitySubpix", _fxaaQualitySubpix);
		ps->SetScalar("fxaaQualityEdgeThreshold", _fxaaQualityEdgeThreshold);
		ps->SetScalar("fxaaQualityEdgeThresholdMin", _fxaaQualityEdgeThresholdMin);
		ps->SetScalar("fxaaConsoleEdgeSharpness", _fxaaConsoleEdgeSharpness);
		ps->SetScalar("fxaaConsoleEdgeThreshold", _fxaaConsoleEdgeThreshold);
		ps->SetScalar("fxaaConsoleEdgeThresholdMin", _fxaaConsoleEdgeThresholdMin);

		ps->SetSRV("fxaaInTex", fxaaInTexRef->Get()->Cast<Texture>()->GetShaderResourceView());

		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ targetTex->GetRenderTargetView(0, 0, 1) });
	}

	PooledTextureRef FXAA::SetupFXAAInTex(const Ptr<Texture> & inTex)
	{
		auto resultTexDesc = inTex->GetDesc();
		resultTexDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, resultTexDesc });

		auto ps = Shader::FindOrCreate<FXAASetupPS>();

		ps->SetSRV("fxaaInTex", inTex->GetShaderResourceView());
		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		DrawQuad({ resultTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
	}
}