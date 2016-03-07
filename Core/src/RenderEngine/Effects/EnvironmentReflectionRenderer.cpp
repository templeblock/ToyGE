#include "ToyGE\RenderEngine\Effects\EnvironmentReflectionRenderer.h"
#include "ToyGE\RenderEngine\ReflectionMap.h"
#include "ToyGE\RenderEngine\Scene.h"

namespace ToyGE
{
	void EnvironmentReflectionRenderer::Render(const Ptr<RenderView> & view)
	{
		auto envMaps = view->GetScene()->GetReflectionMaps();
		auto lut = ReflectionMap::GetLUT();
		auto ambientMap = view->GetScene()->GetAmbientReflectionMap();

		auto sceneColor = view->GetViewRenderContext()->GetSharedTexture("RenderResult");
		auto sceneDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto ssrResult = view->GetViewRenderContext()->GetSharedTexture("SSR");
		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");

		auto newSceneColorRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneColor->GetDesc() });
		auto newSceneColor = newSceneColorRef->Get()->Cast<Texture>();

		std::map<String, String> macros;
		macros["ADD_SSR"] = std::to_string((int)!!ssrResult);
		macros["ADD_AMBIENTMAP"] = std::to_string((int)view->sceneRenderingConfig.bReflectAmbientMap);

		auto cs = Shader::FindOrCreate<EnvironmentReflectionRenderingCS>(macros);
		view->BindShaderParams(cs);
		cs->SetScalar("numCaptures", (uint32_t)view->GetScene()->GetReflectionMapCaptures().size());
		if(envMaps)
			cs->SetScalar("numEnvMapMipLevels", (uint32_t)envMaps->GetDesc().mipLevels);
		else
			cs->SetScalar("numEnvMapMipLevels", (uint32_t)8);
		cs->SetScalar("iblLUTSize", lut->GetTexSize());
		cs->SetScalar("texSize", sceneColor->GetTexSize());

		cs->SetSRV("iblLUT", lut->GetShaderResourceView());
		cs->SetSRV("sceneDepth", sceneDepth->GetShaderResourceView(0, 0, 0, 0, false, RENDER_FORMAT_R24_UNORM_X8_TYPELESS));
		if(envMaps)
			cs->SetSRV("environmentMaps", envMaps->GetShaderResourceView(0, 0, 0, 0, true));
		if (view->sceneRenderingConfig.bReflectAmbientMap && ambientMap && ambientMap->GetPrefiltedEnviromentMap())
			cs->SetSRV("sceneAmbientMap", ambientMap->GetPrefiltedEnviromentMap()->Get()->Cast<Texture>()->GetShaderResourceView(0, 0, 0, 0, true));
		if(envMaps)
			cs->SetSRV("capturesPosRadiusList", view->GetScene()->GetCapturesPosRadiusBuffer()->GetShaderResourceView(0, 0, RENDER_FORMAT_R32G32B32A32_FLOAT));
		if(ssrResult)
			cs->SetSRV("ssrResult", ssrResult->GetShaderResourceView());
		cs->SetSRV("gbuffer0", gbuffer0->GetShaderResourceView());
		cs->SetSRV("gbuffer1", gbuffer1->GetShaderResourceView());
		cs->SetSRV("sceneColor", sceneColor->GetShaderResourceView());
		cs->SetUAV("sceneColorRW", newSceneColor->GetUnorderedAccessView(0, 0, 1));

		cs->SetSampler("linearSampler", SamplerTemplate<>::Get());
		cs->SetSampler("iblSampler", SamplerTemplate<>::Get());

		cs->Flush();

		int32_t numGroupsX = (sceneColor->GetDesc().width + 15) / 16;
		int32_t numGroupsY = (sceneColor->GetDesc().height + 15) / 16;
		Global::GetRenderEngine()->GetRenderContext()->Compute(numGroupsX, numGroupsY, 1);

		Global::GetRenderEngine()->GetRenderContext()->ResetShader(SHADER_CS);

		view->GetViewRenderContext()->SetSharedResource("RenderResult", newSceneColorRef);
	}
}