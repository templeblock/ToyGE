#include "ToyGE\RenderEngine\Font\BitmapFontRenderer.h"
#include "ToyGE\RenderEngine\Mesh.h"
#include "ToyGE\RenderEngine\RenderInput.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderContext.h"

namespace ToyGE
{
	BitmapFontRenderer::BitmapFontRenderer(const Ptr<BitmapFont> & font)
		: _font(font)
	{
		_textRenderInput = Global::GetRenderEngine()->GetRenderFactory()->CreateRenderInput();
		_textRenderInput->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void BitmapFontRenderer::SetText(const WString & text)
	{
		if (_text != text)
		{
			if (_text.size() != text.size())
			{
				if (_textRenderVB)
				{
					auto transientVB = Global::GetRenderEngine()->GetTransientBuffer(TRANSIENTBUFFER_TEXT_VERTEX);
					auto transientIB = Global::GetRenderEngine()->GetTransientBuffer(TRANSIENTBUFFER_TEXT_INDEX);

					transientVB->Free(_textRenderVB);
					transientIB->Free(_textRenderIB);

					_textRenderVB = nullptr;
					_textRenderIB = nullptr;
				}
			}

			_text = text;
			UpdateRenderBuffers();
		}
	}

	void BitmapFontRenderer::Render(const ResourceView & target, const float2 & screenPos, const float2 pixelSize)
	{
		float2 screenSize = float2(
			static_cast<float>(Global::GetRenderEngine()->GetWindow()->Width()),
			static_cast<float>(Global::GetRenderEngine()->GetWindow()->Height()));

		auto scaling = XMMatrixScaling(pixelSize.x / screenSize.x * 2.0f, pixelSize.y / screenSize.y * 2.0f, 0.0f);

		float2 posH = (screenPos / screenSize) * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
		auto trans = XMMatrixTranslation(posH.x, posH.y, 0.0f);

		auto transformXM = XMMatrixMultiply(scaling, trans);
		XMFLOAT4X4 transfom;
		XMStoreFloat4x4(&transfom, transformXM);

		auto fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"Font.xml");

		fx->VariableByName("transform")->AsScalar()->SetValue(&transfom);
		fx->VariableByName("color")->AsScalar()->SetValue(&_color);

		fx->VariableByName("fontTex")->AsShaderResource()->SetValue(_font->GetRenderMapTex()->CreateTextureView(0, 0, 0, 0));

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderInput(_textRenderInput);
		rc->SetDepthStencil(ResourceView());

		rc->SetRenderTargets({ target }, 0);

		fx->TechniqueByName("FontRender")->PassByIndex(0)->Bind();
		rc->DrawIndexed(static_cast<int32_t>(_text.size()) * 6, 0, 0);
		fx->TechniqueByName("FontRender")->PassByIndex(0)->UnBind();
	}

	void BitmapFontRenderer::UpdateRenderBuffers()
	{
		if (_text.size() == 0)
		{
			_textRenderInput->SetVerticesBuffers({});
			_textRenderInput->SetIndicesBuffers(nullptr);
			return;
		}

		VertexDataBuildHelp builder;
		builder.AddElementDesc("POSITION", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.AddElementDesc("TEXCOORD", 0, RENDER_FORMAT_R32G32B32_FLOAT, 0);
		builder.SetNumVertices(4 * static_cast<int32_t>(_text.size()));

		std::vector<uint32_t> indicesData(6 * static_cast<int32_t>(_text.size()));

		float3 curPos = 0.0f;
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		builder.Start();

		auto transientVB = Global::GetRenderEngine()->GetTransientBuffer(TRANSIENTBUFFER_TEXT_VERTEX);
		auto transientIB = Global::GetRenderEngine()->GetTransientBuffer(TRANSIENTBUFFER_TEXT_INDEX);
		if (_textRenderVB == nullptr)
		{
			_textRenderVB = transientVB->Alloc(builder.vertexDataDesc.numVertices * builder.vertexDataDesc.vertexByteSize);
			_textRenderIB = transientIB->Alloc(static_cast<int32_t>(sizeof(uint32_t) * indicesData.size()));
		}

		for (auto & curChar : _text)
		{
			auto glyphIndex = _font->GetCharGlyphIndex(curChar);
			auto & glyphRenderInfo = _font->GetBitmapFontGlyphRenderInfo(glyphIndex);

			float left = curPos.x + glyphRenderInfo.glyphBearingX;
			float right = left + glyphRenderInfo.glyphWidth;
			float top = curPos.y + glyphRenderInfo.glyphBearingY;
			float bottom = curPos.y - (glyphRenderInfo.glyphHeight - glyphRenderInfo.glyphBearingY);

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

			curPos.x += glyphRenderInfo.glyphAdvanceWidth;
		}
		builder.Finish();

		transientVB->Update(_textRenderVB, builder.vertexDataDesc.pData.get(), _textRenderVB->bytesSize, 0);
		transientVB->UpdateFinish();

		transientIB->Update(_textRenderIB, &indicesData[0], _textRenderIB->bytesSize, 0);
		transientIB->UpdateFinish();

		transientVB->GetBuffer()->SetVertexBufferType(VERTEX_BUFFER_GEOMETRY);
		transientVB->GetBuffer()->SetVertexElementsDesc(builder.vertexDataDesc.elementsDesc);

		_textRenderInput->SetVerticesBuffers({ transientVB->GetBuffer() });
		_textRenderInput->SetVerticesBytesOffset(0, _textRenderVB->bytesOffset);

		_textRenderInput->SetIndicesBuffers(transientIB->GetBuffer());
		_textRenderInput->SetIndicesBytesOffset(_textRenderIB->bytesOffset);

		//_textRenderInput->SetVerticesElementDesc(0, builder.vertexDataDesc.elementsDesc);

		/*RenderBufferDesc vertexBufDesc;
		vertexBufDesc.bindFlag = BUFFER_BIND_VERTEX | BUFFER_BIND_IMMUTABLE;
		vertexBufDesc.cpuAccess = 0;
		vertexBufDesc.numElements = builder.vertexDataDesc.numVertices;
		vertexBufDesc.elementSize = builder.vertexDataDesc.vertexByteSize;
		vertexBufDesc.structedByteStride = 0;

		auto vb = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(
			vertexBufDesc, builder.vertexDataDesc.pData.get(), VERTEX_BUFFER_GEOMETRY, builder.vertexDataDesc.elementsDesc);
		_textRenderInput->SetVerticesBuffers({ vb });

		RenderBufferDesc indexBufDesc;
		indexBufDesc.bindFlag = BUFFER_BIND_INDEX | BUFFER_BIND_IMMUTABLE;
		indexBufDesc.cpuAccess = 0;
		indexBufDesc.numElements = static_cast<int32_t>(indicesData.size());
		indexBufDesc.elementSize = sizeof(indicesData[0]);
		indexBufDesc.structedByteStride = 0;

		auto ib = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer(
			indexBufDesc, &indicesData[0]);
		_textRenderInput->SetIndicesBuffers(ib);*/
	}
}
