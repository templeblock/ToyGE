#include "ToyGE\RenderEngine\Font\SignedDistanceFontRenderer.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	SignedDistanceFontRenderer::SignedDistanceFontRenderer(const Ptr<SignedDistanceFieldFont> & font)
		: BitmapFontRenderer(font)
	{

	}

	void SignedDistanceFontRenderer::Render(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width)
	{
		if (_text.size() == 0)
			return;

		if (_bNeedUpdate)
			UpdateRenderBuffers();

		auto fontRenderingVS = Shader::FindOrCreate<FontRenderingVS>();
		auto fontRenderingPS = Shader::FindOrCreate<SignedDistanceFontRenderingPS>();

		fontRenderingVS->SetScalar("transform", ComputeTransform(target, screenPos, height, width));

		fontRenderingPS->SetScalar("color", _color);
		fontRenderingPS->SetSRV("fontTex", _font->GetGlyphRenderMapTex()->GetShaderResourceView(0, 0, 0, 0));
		fontRenderingPS->SetSampler("bilinearSampler", SamplerTemplate<>::Get());

		fontRenderingVS->Flush();
		fontRenderingPS->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_SRC_ALPHA, BLEND_PARAM_INV_SRC_ALPHA>::Get());

		rc->SetVertexBuffer({ _vb }, { _vbTransientSubAlloc->bytesOffset });
		rc->SetIndexBuffer(_ib, _ibTransientSubAlloc->bytesOffset);
		rc->SetPrimitiveTopology(PrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		rc->SetRenderTargets({ target });
		rc->SetDepthStencil(nullptr);

		rc->SetViewport(GetTextureQuadViewport(target->GetResource()->Cast<Texture>()));

		rc->DrawIndexed(_textNumCharacters * 6, 0, 0);

		rc->SetBlendState(nullptr);
	}
}