#include "ToyGE\RenderEngine\Font\BitmapFontRenderer.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderUtil.h"

namespace ToyGE
{
	BitmapFontRenderer::BitmapFontRenderer(const Ptr<BitmapFont> & font)
		: _font(font)
	{
	}

	void BitmapFontRenderer::SetText(const String & text)
	{
		if (_text != text)
		{
			_bNeedUpdate = true;

			if (_text.size() != text.size())
			{
				if (_vbTransientSubAlloc)
				{
					_vbTransientSubAlloc->Free();
					_vbTransientSubAlloc = nullptr;
					_ibTransientSubAlloc->Free();
					_ibTransientSubAlloc = nullptr;
				}
			}

			_text = text;
		}
	}

	void BitmapFontRenderer::Render(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width)
	{
		if (_text.size() == 0)
			return;

		if(_bNeedUpdate)
			UpdateRenderBuffers();

		auto fontRenderingVS = Shader::FindOrCreate<FontRenderingVS>();
		auto fontRenderingPS = Shader::FindOrCreate<BitmapFontRenderingPS>();
		
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

	void BitmapFontRenderer::UpdateRenderBuffers()
	{
		auto u32Text = StringConvert<StringEncode::UTF_8, StringEncode::UTF_32>(_text);
		_textNumCharacters = (int32_t)u32Text.size();

		// Init buffers data
		VertexBufferBuilder builder;
		builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc("TEXCOORD", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.SetNumVertices(4 * static_cast<int32_t>(u32Text.size()));

		std::vector<uint32_t> indicesData(6 * static_cast<int32_t>(u32Text.size()));

		float3 curPos = 0.0f;
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		for (auto & curChar : u32Text)
		{
			auto glyphIndex = _font->GetAsset()->GetCharGlyphIndex(curChar);
			auto & glyphRenderInfo = _font->GetBitmapFontGlyphRenderInfo(glyphIndex);

			float left = curPos.x() + glyphRenderInfo.glyphBearingX;
			float right = left + glyphRenderInfo.glyphWidth;
			float top = curPos.y() + glyphRenderInfo.glyphBearingY;
			float bottom = curPos.y() - (glyphRenderInfo.glyphHeight - glyphRenderInfo.glyphBearingY);

			builder.Add(float3(left, top, 0.0f));
			builder.Add(float3(glyphRenderInfo.renderMapUVLeft, glyphRenderInfo.renderMapUVTop, glyphRenderInfo.renderMapSlice));

			builder.Add(float3(right, top, 0.0f));
			builder.Add(float3(glyphRenderInfo.renderMapUVRight, glyphRenderInfo.renderMapUVTop, glyphRenderInfo.renderMapSlice));

			builder.Add(float3(left, bottom, 0.0f));
			builder.Add(float3(glyphRenderInfo.renderMapUVLeft, glyphRenderInfo.renderMapUVBottom, glyphRenderInfo.renderMapSlice));

			builder.Add(float3(right, bottom, 0.0f));
			builder.Add(float3(glyphRenderInfo.renderMapUVRight, glyphRenderInfo.renderMapUVBottom, glyphRenderInfo.renderMapSlice));

			indicesData[indexOffset++] = vertexOffset;
			indicesData[indexOffset++] = vertexOffset + 1;
			indicesData[indexOffset++] = vertexOffset + 2;

			indicesData[indexOffset++] = vertexOffset + 1;
			indicesData[indexOffset++] = vertexOffset + 3;
			indicesData[indexOffset++] = vertexOffset + 2;

			vertexOffset += 4;

			curPos.x() += glyphRenderInfo.glyphAdvanceWidth;
		}

		// Init buffers
		auto transientVB = Global::GetTransientBuffer(TRANSIENTBUFFER_TEXT_VERTEX);
		if (_vbTransientSubAlloc == nullptr)
		{
			_vbTransientSubAlloc = transientVB->Alloc(builder.numVertices * builder.vertexSize);
		}
		transientVB->Update(_vbTransientSubAlloc, builder.dataBuffer.get(), _vbTransientSubAlloc->bytesSize, 0);
		transientVB->FlushUpdate();
		_vb = transientVB->GetBuffer()->DyCast<VertexBuffer>();
		_vb->SetType(VERTEX_BUFFER_GEOMETRY);
		_vb->SetElementsDesc(builder.vertexElementsDesc);

		auto transientIB = Global::GetTransientBuffer(TRANSIENTBUFFER_TEXT_INDEX);
		if (_ibTransientSubAlloc == nullptr)
		{
			_ibTransientSubAlloc = transientIB->Alloc(static_cast<int32_t>(sizeof(uint32_t) * indicesData.size()));
		}
		transientIB->Update(_ibTransientSubAlloc, &indicesData[0], _ibTransientSubAlloc->bytesSize, 0);
		transientIB->FlushUpdate();
		_ib = transientIB->GetBuffer();

		_bNeedUpdate = false;
	}

	float4x4 BitmapFontRenderer::ComputeTransform(const Ptr<RenderTargetView> & target, const float2 & screenPos, float height, float width)
	{
		float2 targetSize = float2(
			static_cast<float>(target->GetResource()->Cast<Texture>()->GetDesc().width),
			static_cast<float>(target->GetResource()->Cast<Texture>()->GetDesc().height));

		float scalingY = height / targetSize.y() * 2.0f;
		float scalingX;
		if (width == 0.0f)
			scalingX = height / targetSize.x() * 2.0f;
		else
			scalingX = width / targetSize.x() * 2.0f;
		auto scalingMat = scaling(float3(scalingX, scalingY, 0.0f));

		float2 posH = (screenPos / targetSize) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
		auto transMat = translation(posH.x(), posH.y(), 0.0f);

		/*auto transformXM = XMMatrixMultiply(scaling, trans);
		XMFLOAT4X4 transform;
		XMStoreFloat4x4(&transform, transformXM);*/

		return mul(scalingMat, transMat);
	}
}
