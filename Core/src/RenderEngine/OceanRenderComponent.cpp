#include "ToyGE\RenderEngine\OceanRenderComponent.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\RenderEngineInclude.h"
#include "ToyGE\RenderEngine\FFT.h"

namespace ToyGE
{
	OceanTileRenderComponent::OceanTileRenderComponent()
	{
		_bCastShadows = false;
		_bSpecialRender = true;
	}

	void OceanTileRenderComponent::Init()
	{
		_tileMesh = CommonMesh::CreatePlane(_width, _height, _tileSize - 1, _tileSize - 1, false, false);
	}

	AABBox OceanTileRenderComponent::GetBoundsAABB() const
	{
		float3 center = GetWorldPos();
		float3 extent = float3(_width * 0.5f, _height * 0.5f, 10.0f);
		float3 min = center - extent;
		float3 max = center + extent;
		return AABBox(min, max);
	}

	void OceanTileRenderComponent::SpecialRender(const Ptr<class RenderView> & view)
	{
		auto gbuffer0 = view->GetViewRenderContext()->GetSharedTexture("GBuffer0");
		auto gbuffer1 = view->GetViewRenderContext()->GetSharedTexture("GBuffer1");
		auto gbuffer2 = view->GetViewRenderContext()->GetSharedTexture("GBuffer2");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		for (auto & light : view->GetViewRenderContext()->lights)
		{
			std::map<String, String> macros;

			light->BindMacros(true, view, macros);
			auto meshElement = _tileMesh->GetRenderData()->GetMeshElements()[0];
			if (meshElement)
				meshElement->BindMacros(macros);


			auto vs = Shader::FindOrCreate<OceanShadingVS>(macros);
			auto ps = Shader::FindOrCreate<OceanShadingPS>(macros);

			view->BindShaderParams(vs);
			view->BindShaderParams(ps);

			BindShaderParams(vs);
			light->BindShaderParams(ps, true, view);

			vs->SetScalar("tileLength", float2(_width, _height));

			ps->SetScalar("gridLen", _width / (float)_tileSize);
			ps->SetScalar("ambientColor", view->GetScene()->GetAmbientColor());
			ps->SetScalar("frameCount", (uint32_t)Global::GetInfo()->frameCount);

			vs->SetSRV("displacementMap", _displacementMap->GetShaderResourceView());
			vs->SetSampler("bilinearSampler",
				SamplerTemplate<
				FILTER_MIN_MAG_MIP_LINEAR,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP >::Get());

			ps->SetSRV("normalFoldingMap", _normalFoldingMap->GetShaderResourceView());
			ps->SetSampler("bilinearSampler",
				SamplerTemplate<
				FILTER_MIN_MAG_MIP_LINEAR,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP,
				TEXTURE_ADDRESS_WRAP >::Get());

			vs->Flush();
			ps->Flush();

			rc->SetDepthStencilState(
				DepthStencilStateTemplate<
				true,
				DEPTH_WRITE_ALL,
				COMPARISON_LESS,
				true, 0xff, 0xff,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				STENCIL_OP_REPLACE,
				COMPARISON_ALWAYS>::Get(), 1);

			//rc->SetRasterizerState(RasterizerStateTemplate<FILL_WIREFRAME>::Get());

			rc->SetRenderTargets({
				sceneTex->GetRenderTargetView(0, 0, 1),
				gbuffer0->GetRenderTargetView(0, 0, 1),
				gbuffer1->GetRenderTargetView(0, 0, 1),
				gbuffer2->GetRenderTargetView(0, 0, 1)
			});
			rc->SetDepthStencil(sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

			meshElement->Draw();

			rc->SetRasterizerState(nullptr);
		}
	}


	static const float gravity = 9.8f;
	float Phillips(float2 k, float2 windDir, float windSpeed, float amplitude, float dirDepend)
	{
		float L = windSpeed * windSpeed / gravity;

		float k2 = dot(k, k);
		if (equal(k2, 0.0f))
			return 0.0f;
		float k_dot_w = dot(k, windDir);
		float phi = amplitude * exp(-1.0f / (k2 * L * L)) / (k2 * k2 * k2) * (k_dot_w * k_dot_w);

		float l = L / 1000.0f;

		phi *= exp(-k2 * l * l);

		if (k_dot_w < 0)
			phi *= dirDepend;

		return phi;
	}

	float GaussRandom()
	{
		float u1 = rand() / (float)RAND_MAX;
		float u2 = rand() / (float)RAND_MAX;
		if (u1 < 1e-6f)
			u1 = 1e-6f;
		return sqrtf(-2 * logf(u1)) * cosf(PI2 * u2);
	}

	OceanRenderComponent::OceanRenderComponent()
	{
		_tileWidth = 2000.0f;
		_tileHeight = 2000.0f;
		_tileSize = 512;
		_numTiles = 1;

		for (int y = 0; y < _numTiles.y(); ++y)
		{
			for (int x = 0; x < _numTiles.x(); ++x)
			{
				auto tile = std::make_shared<OceanTileRenderComponent>();
				_tiles.push_back(tile);
			}
		}
	}

	void OceanRenderComponent::Init()
	{
		float2 windDir = normalize( float2(0.8f, 0.6f) );
		int32_t mapSize = 512;
		float dirDepend = 0.07f;
		float windSpeed = 60.0f;
		float amplitude = 0.35f * 1e-7;

		// Compute h0 & omega data
		srand(0);
		std::vector<float2> h0Data;
		std::vector<float> omegaData;
		static const float inv_sqrt2 = 1.0f / sqrt(2.0f);
		for (int y = 0; y < mapSize; ++y)
		{
			float2 k;
			k.y() = (float)(y - mapSize / 2) * PI2 / _tileHeight;
			for (int x = 0; x < mapSize; ++x)
			{
				k.x() = (float)(x - mapSize / 2) * PI2 / _tileWidth;

				float2 h0;
				h0.x() = inv_sqrt2 * GaussRandom() * sqrt(Phillips(k, windDir, windSpeed, amplitude, dirDepend));
				h0.y() = inv_sqrt2 * GaussRandom() * sqrt(Phillips(k, windDir, windSpeed, amplitude, dirDepend));
				h0Data.push_back(h0);

				float omega = sqrt(gravity * length(k));
				omegaData.push_back(omega);
			}
		}

		// Init texture
		TextureDesc texDesc;
		texDesc.width = mapSize;
		texDesc.height = mapSize;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.bCube = false;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS | TEXTURE_BIND_GENERATE_MIPS;
		texDesc.cpuAccess = 0;
		texDesc.mipLevels = 1;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		RenderDataDesc dataDesc;
		dataDesc.pData = &h0Data[0];
		dataDesc.rowPitch = sizeof(float2) * (size_t)texDesc.width;
		dataDesc.slicePitch = dataDesc.rowPitch * (size_t)texDesc.height;

		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		_h0 = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_h0->SetDesc(texDesc);
		_h0->Init({ dataDesc });

		dataDesc.pData = &omegaData[0];
		dataDesc.rowPitch = sizeof(float) * (size_t)texDesc.width;
		dataDesc.slicePitch = dataDesc.rowPitch * (size_t)texDesc.height;

		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		_omega = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_omega->SetDesc(texDesc);
		_omega->Init({ dataDesc });

		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		_displacementMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_displacementMap->SetDesc(texDesc);
		_displacementMap->Init({ dataDesc });

		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		_normalFoldingMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_normalFoldingMap->SetDesc(texDesc);
		_normalFoldingMap->Init({ dataDesc });

		for (auto & tile : _tiles)
		{
			tile->SetWidth(_tileWidth);
			tile->SetHeight(_tileHeight);
			tile->SetTileSize(_tileSize);
			tile->Init();
			tile->AttachTo(Cast<TransformComponent>());
		}

		Tick(0.0f);
	}

	void OceanRenderComponent::Tick(float elapsedTime)
	{
		if (!_h0 || !_omega)
			return;

		static float time = 0.0f;

		auto texDesc = _h0->GetDesc();
		auto htRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto ht = htRef->Get()->Cast<Texture>();
		auto dtxRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dtx = dtxRef->Get()->Cast<Texture>();
		auto dtzRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dtz = dtzRef->Get()->Cast<Texture>();

		{
			time += elapsedTime;

			auto updateDisplacementPS = Shader::FindOrCreate<UpdateDisplacementMapPS>();
			updateDisplacementPS->SetScalar("texSize", _h0->GetTexSize());
			updateDisplacementPS->SetScalar("time", time);
			updateDisplacementPS->SetSRV("h0Tex", _h0->GetShaderResourceView());
			updateDisplacementPS->SetSRV("omegaTex", _omega->GetShaderResourceView());
			updateDisplacementPS->Flush();

			DrawQuad({
				ht->GetRenderTargetView(0, 0, 1),
				dtx->GetRenderTargetView(0, 0, 1),
				dtz->GetRenderTargetView(0, 0, 1) });
		}

		// IFFT
		auto dyRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dy = dyRef->Get()->Cast<Texture>();
		auto dxRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dx = dxRef->Get()->Cast<Texture>();
		auto dzRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto dz = dzRef->Get()->Cast<Texture>();

		FFT::FFT2DRadix8(ht,  0, 0, dy, 0, 0, true, false);
		FFT::FFT2DRadix8(dtx, 0, 0, dx, 0, 0, true, false);
		FFT::FFT2DRadix8(dtz, 0, 0, dz, 0, 0, true, false);

		// Gen displacement map
		{
			auto ps = Shader::FindOrCreate<MergeDisplacementMapPS>();
			ps->SetSRV("dxTex", dx->GetShaderResourceView());
			ps->SetSRV("dyTex", dy->GetShaderResourceView());
			ps->SetSRV("dzTex", dz->GetShaderResourceView());
			ps->Flush();

			DrawQuad({ _displacementMap->GetRenderTargetView(0, 0, 1) });
			_displacementMap->GenerateMips();
		}

		// Gen normal & folding
		{
			auto ps = Shader::FindOrCreate<GenNormalFoldingPS>();
			ps->SetScalar("texSize", _displacementMap->GetTexSize());
			ps->SetScalar("gridLen", 512.0f / _tileWidth);
			ps->SetScalar("gridSize", _tileWidth / 512.0f * 2.0f);
			ps->SetSRV("displacementMap", _displacementMap->GetShaderResourceView());
			ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
			ps->Flush();

			DrawQuad({ _normalFoldingMap->GetRenderTargetView(0, 0, 1) });
			_normalFoldingMap->GenerateMips();
		}

		for (auto & tile : _tiles)
		{
			tile->SetDisplacementMap(_displacementMap);
			tile->SetNormalFoldingMap(_normalFoldingMap);
		}
	} 

	void OceanRenderComponent::Activate()
	{
		TransformComponent::Activate();
		for (auto & tile : _tiles)
			tile->Activate();
	}

}