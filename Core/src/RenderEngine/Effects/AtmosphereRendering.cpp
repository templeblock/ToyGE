#include "ToyGE\RenderEngine\Effects\AtmosphereRendering.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\Scene.h"

namespace ToyGE
{
	DECLARE_SHADER(, RenderSampleCoordsVS, SHADER_VS, "AtmosphereRendering", "RenderSampleCoordsVS", SM_4);
	DECLARE_SHADER(, RenderSampleCoordsPS, SHADER_PS, "AtmosphereRendering", "RenderSampleCoordsPS", SM_4);

	AtmosphereRendering::AtmosphereRendering()
		: _numSampleLines(1024),
		_maxSamplesPerLine(1024),
		_initalSampleStep(4),
		_earthRadius(6360000.0f),
		_atmosphereTopHeight(80000.0f),
		_particleScaleHeightR(7994.0f),
		_particleScaleHeightM(1200.0f),
		_phaseG_M(0.76f),
		_sunDirection(0.0f, 1.0f, 0.0f),
		_sunRadiance(1.0f, 1.0f, 1.0f),
		_sunRenderColor(1.0f, 1.0f, 1.0f),
		_sunRenderRadius(120.0f)
	{
		_scatteringR = float3(5.8f, 13.5f, 33.1f) * 1e-6f;
		_scatteringM = float3(2.0f) * 1e-5f;
		_attenuationR = _scatteringR;
		_attenuationM = 1.1f * _scatteringM;

		bEpipolarSampling = true;
	}

	void AtmosphereRendering::Render(const Ptr<RenderView> & view)
	{
		if (!_opticalDepthLUT)
			InitOpticalDepthLUT();
		if (!_inScatteringLUTR)
			InitInScatteringLUT();

		auto sceneLinearClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneLinearClipDepth");
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");;
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		float2 targetSize = view->GetViewParams().viewSize;

		auto & cameraPos = view->GetCamera()->GetPos();
		float sunDist = 1e+2;
		auto sunPosW = cameraPos - _sunDirection * sunDist;
		auto sunPosV= transform_coord(sunPosW, view->GetCamera()->GetViewMatrix());
		float4 sunPosH = 0.0f;
		if (sunPosV.z() == 0.0f)
		{
			sunPosH = sunPosV;
			float scale = 2.0f / max(abs(sunPosH.x()), abs(sunPosH.y()));
			sunPosH.x() *= scale;
			sunPosH.y() *= scale;
		}
		else
		{
			sunPosH = transform_coord(sunPosV, view->GetCamera()->GetProjMatrix());
			/*if (abs(sunPosH.x()) > 1.0f || abs(sunPosH.y()) > 1.0f)
			{
				float scale = 2.0f / max(abs(sunPosH.x()), abs(sunPosH.y()));
				sunPosH.x() *= scale;
				sunPosH.y() *= scale;
			}*/
		}

		if (bEpipolarSampling)
		{
			// Init sample lines
			auto sampleLinesTexRef = InitSampleLines(sceneTex->GetTexSize(), float2(sunPosH.x(), sunPosH.y()));
			auto sampleLinesTex = sampleLinesTexRef->Get()->Cast<Texture>();

			// Init samples
			PooledTextureRef sampleCoordsTexRef, sampleDepthTexRef, sampleMaskDSRef;
			InitSampleCoordsTex(
				sceneTex->GetTexSize(),
				sampleLinesTex,
				sceneLinearClipDepth,
				sampleCoordsTexRef,
				sampleDepthTexRef,
				sampleMaskDSRef);
			auto sampleCoordsTex = sampleCoordsTexRef->Get()->Cast<Texture>();
			auto sampleDepthTex = sampleDepthTexRef->Get()->Cast<Texture>();
			auto sampleMaskDS = sampleMaskDSRef->Get()->Cast<Texture>();

			// Refine samples
			float depthBreakThreshold = 0.5f / (view->GetCamera()->GetFar() - view->GetCamera()->GetNear());
			auto interpolationSourceTexRef =
				RefineSamples(
					sceneTex->GetTexSize(),
					depthBreakThreshold,
					sampleCoordsTex,
					sampleDepthTex);
			auto interpolationSourceTex = interpolationSourceTexRef->Get()->Cast<Texture>();

			// Mark samples
			MarkRayMarchingSamples(interpolationSourceTex, sampleMaskDS);

			// Ray marching
			PooledTextureRef lightAccumTexRef, attenuationTexRef;
			DoRayMarching(
				view,
				sampleCoordsTex,
				sampleDepthTex,
				sampleMaskDS,
				lightAccumTexRef,
				attenuationTexRef);
			auto lightAccumTex = lightAccumTexRef->Get()->Cast<Texture>();
			auto attenuationTex = attenuationTexRef->Get()->Cast<Texture>();

			// Interpolate rest samples
			PooledTextureRef lightAccumInterpTexRef, attenuationInterpTexRef;
			InterpolateRestSamples(
				sceneTex->GetTexSize(),
				interpolationSourceTex,
				sampleDepthTex,
				lightAccumTex,
				attenuationTex,
				sampleMaskDS,
				lightAccumInterpTexRef,
				attenuationInterpTexRef);
			auto lightAccumInterpTex = lightAccumInterpTexRef->Get()->Cast<Texture>();
			auto attenuationInterpTex = attenuationInterpTexRef->Get()->Cast<Texture>();

			auto targetTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, sceneTex->GetDesc() });
			auto targetTex = targetTexRef->Get()->Cast<Texture>();

			// Unwarp samples
			UnWarpSamples(
				float2(sunPosH.x(), sunPosH.y()),
				lightAccumInterpTex,
				attenuationInterpTex,
				sampleLinesTex,
				sampleDepthTex,
				sceneTex,
				sceneLinearClipDepth,
				targetTex->GetRenderTargetView(0, 0, 1));

			// Render sun
			if (sunPosV.z() > 0.0f)
			{
				if (sceneClipDepth)
				{
					DoRenderSun(
						float2(sunPosH.x(), sunPosH.y()),
						view,
						targetTex->GetRenderTargetView(0, 0, 1),
						sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
				}
				else
				{
					DoRenderSun(
						float2(sunPosH.x(), sunPosH.y()),
						view,
						targetTex->GetRenderTargetView(0, 0, 1),
						nullptr);
				}
			}

			view->GetViewRenderContext()->SetSharedResource("RenderResult", targetTexRef);
		}
		else
		{
			// Ray marching
			PooledTextureRef lightAccumTexRef, attenuationTexRef;
			DoRayMarching(
				view,
				nullptr,
				sceneLinearClipDepth,
				nullptr,
				lightAccumTexRef,
				attenuationTexRef);
			auto lightAccumTex = lightAccumTexRef->Get()->Cast<Texture>();
			auto attenuationTex = attenuationTexRef->Get()->Cast<Texture>();

			// Accum ray marching results
			AccumRayMarching(lightAccumTex, attenuationTex, sceneTex->GetRenderTargetView(0, 0, 1));

			// Render sun
			if (sunPosV.z() > 0.0f)
			{
				if (sceneClipDepth)
				{
					DoRenderSun(
						float2(sunPosH.x(), sunPosH.y()),
						view,
						sceneTex->GetRenderTargetView(0, 0, 1),
						sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
				}
				else
				{
					DoRenderSun(
						float2(sunPosH.x(), sunPosH.y()),
						view,
						sceneTex->GetRenderTargetView(0, 0, 1),
						nullptr);
				}
			}
		}
		
	}

	void AtmosphereRendering::RenderSun(const Ptr<RenderView> & view)
	{
		auto sceneClipDepth = view->GetViewRenderContext()->GetSharedTexture("SceneClipDepth");;
		auto sceneTex = view->GetViewRenderContext()->GetSharedTexture("RenderResult");

		auto & cameraPos = view->GetCamera()->GetPos();
		float sunDist = 1e+2;
		auto sunPosW = cameraPos - _sunDirection * sunDist;
		auto sunPosV = transform_coord(sunPosW, view->GetCamera()->GetViewMatrix());
		float4 sunPosH = 0.0f;
		if (sunPosV.z() == 0.0f)
		{
			sunPosH = sunPosV;
			float scale = 2.0f / max(abs(sunPosH.x()), abs(sunPosH.y()));
			sunPosH.x() *= scale;
			sunPosH.y() *= scale;
		}
		else
		{
			sunPosH = transform_coord(sunPosV, view->GetCamera()->GetProjMatrix());
		}

		if (sunPosV.z() > 0.0f)
		{
			if (sceneClipDepth)
			{
				DoRenderSun(
					float2(sunPosH.x(), sunPosH.y()),
					view,
					sceneTex->GetRenderTargetView(0, 0, 1),
					sceneClipDepth->GetDepthStencilView(0, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));
			}
			else
			{
				DoRenderSun(
					float2(sunPosH.x(), sunPosH.y()),
					view,
					sceneTex->GetRenderTargetView(0, 0, 1),
					nullptr);
			}
		}
	}

	void AtmosphereRendering::RenderCubeMap(const Ptr<Texture> & target)
	{
		bool bEpipolarSave = bEpipolarSampling;
		bEpipolarSampling = false;

		static std::vector<float3> viewDir =
		{
			float3(1.0f, 0.0f, 0.0f),
			float3(-1.0f, 0.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, -1.0f, 0.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 0.0f, -1.0f)
		};
		static std::vector<float3> upDir =
		{
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 0.0f, -1.0f),
			float3(0.0f, 0.0f, 1.0f),
			float3(0.0f, 1.0f, 0.0f),
			float3(0.0f, 1.0f, 0.0f)
		};

		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = (float)target->GetDesc().width;
		vp.height = (float)target->GetDesc().height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		auto view = std::make_shared<RenderView>();
		view->SetViewport(vp);

		auto camera = std::make_shared<PerspectiveCamera>(PI_DIV2, 1.0f, 0.1f, 100.0f);
		view->SetCamera(camera);

		for (int i = 0; i < 6; ++i)
		{
			auto texDesc = target->GetDesc();
			texDesc.bCube = false;
			texDesc.arraySize = 1;
			texDesc.mipLevels = 1;
			auto tmpTargetRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
			auto tmpTarget = tmpTargetRef->Get()->Cast<Texture>();

			view->GetViewRenderContext()->SetSharedResource("RenderResult", tmpTargetRef);
			view->SetRenderTarget(tmpTarget->GetRenderTargetView(0, 0, 1));

			Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(tmpTarget->GetRenderTargetView(0, 0, 1), 0.0f);

			camera->LookTo(0.0f, viewDir[i], upDir[i]);
			view->UpdateParamsBuffer();

			Render(view);

			ToyGE_ASSERT( tmpTarget->CopyTo(target, 0, i, 0, 0, 0, 0, 0) );
		}
		target->GenerateMips();

		bEpipolarSampling = bEpipolarSave;
	}

	void AtmosphereRendering::RenderHemiPanoramicMap(const Ptr<Texture> & target)
	{
		auto ps = Shader::FindOrCreate<RenderHemiPanoramicMapPS>();
		ps->SetScalar("lightRadiance", _sunRadiance);
		ps->SetScalar("lightDirection", _sunDirection);

		ps->SetScalar("lutSize", float4((float)_lutSize.x(), (float)_lutSize.y(), (float)_lutSize.z(), (float)_lutSize.w()));

		ps->SetScalar("earthCenter", float3(0.0f, -_earthRadius, 0.0f));
		ps->SetScalar("earthRadius", _earthRadius);
		ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);

		ps->SetScalar("particleScaleHeight", float2(_particleScaleHeightR, _particleScaleHeightM));
		ps->SetScalar("scatteringR", _scatteringR);
		ps->SetScalar("scatteringM", _scatteringM);
		ps->SetScalar("attenuationR", _attenuationR);
		ps->SetScalar("attenuationM", _attenuationM);
		ps->SetScalar("phaseG_M", _phaseG_M);

		ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());
		ps->SetSRV("inScatteringLUTR", _inScatteringLUTR->GetShaderResourceView());
		ps->SetSRV("inScatteringLUTM", _inScatteringLUTM->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ target->GetRenderTargetView(0, 0, 1) });

		target->GenerateMips();
	}

	void AtmosphereRendering::UpdateSunLight(const Ptr<class DirectionalLightComponent> & light)
	{
		auto radiance = ComputeSunRadianceAt(_sunDirection, _sunRadiance, 1.0f);
		auto maxV = std::max<float>(std::max<float>(radiance.x(), radiance.y()), radiance.z());
		if (maxV > 1e-4f)
		{
			float3 color = radiance / maxV;
			light->SetColor(color);
			light->SetIntensity(maxV);
		}
		else
		{
			light->SetColor(0.0f);
			light->SetIntensity(0.0f);
		}
		light->SetDirection(_sunDirection);
	}

	void AtmosphereRendering::UpdateAmbientAndReflectionMap(const Ptr<class Scene> & scene)
	{
		static Ptr<Texture> ambientCubeMap;
		static Ptr<Texture> ambientPanoramicMap;
		if (!ambientCubeMap)
		{
			// Init target texture
			TextureDesc texDesc;
			texDesc.width = texDesc.height = 512;
			texDesc.depth = 1;
			texDesc.arraySize = 1;
			texDesc.bCube = true;
			texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_GENERATE_MIPS;
			texDesc.cpuAccess = 0;
			texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
			texDesc.mipLevels = 0;
			texDesc.sampleCount = 1;
			texDesc.sampleQuality = 0;

			ambientCubeMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
			ambientCubeMap->SetDesc(texDesc);
			ambientCubeMap->Init();

			texDesc.width = 2048;
			texDesc.height = 2048;
			texDesc.bCube = false;
			ambientPanoramicMap = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
			ambientPanoramicMap->SetDesc(texDesc);
			ambientPanoramicMap->Init();
		}

		RenderCubeMap(ambientCubeMap);
		RenderHemiPanoramicMap(ambientPanoramicMap);

		scene->SetAmbientMapTexture(ambientPanoramicMap, AM_HEMIPANORAMIC);
		scene->UpdateAmbientReflectionMap(ambientCubeMap);
	}

	float3 AtmosphereRendering::ComputeSunRadianceAt(const float3 & sunDir, const float3 & sunRadiance, float height)
	{
		if (!_opticalDepthLUT)
			InitOpticalDepthLUT();

		TextureDesc texDesc;
		texDesc.width = texDesc.height = 1;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		//texDesc.type = TEXTURE_2D;

		auto radianceTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto radianceTex = radianceTexRef->Get()->Cast<Texture>();

		auto ps = Shader::FindOrCreate<ComputeSunRadiancePS>();

		ps->SetScalar("lightRadiance", sunRadiance);
		ps->SetScalar("lightDirection", sunDir);
		ps->SetScalar("computeSunRadianceHeight", height);

		ps->SetScalar("attenuationR", _attenuationR);
		ps->SetScalar("attenuationM", _attenuationM);

		ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);

		ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ radianceTex->GetRenderTargetView(0, 0, 1) });

		/*_fx->VariableByName("opticalDepthLUT")->AsShaderResource()->SetValue(_opticalDepthLUT->CreateTextureView());

		_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&sunRadiance);
		_fx->VariableByName("lightDirection")->AsScalar()->SetValue(&sunDir);
		_fx->VariableByName("computeSunRadianceHeight")->AsScalar()->SetValue(&height);

		_fx->VariableByName("attenuationR")->AsScalar()->SetValue(&_attenuationR);
		_fx->VariableByName("attenuationM")->AsScalar()->SetValue(&_attenuationM);

		_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&atmosphereTopHeight);*/

		/*auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ radianceTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("ComputeSunRadiance"), 0, 0, 1, 1);*/

		texDesc.bindFlag = 0;
		texDesc.cpuAccess = CPU_ACCESS_READ;
		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto resultTex = resultTexRef->Get()->Cast<Texture>();

		radianceTex->CopyTo(resultTex, 0, 0, 0, 0, 0, 0, 0);

		auto mapData = resultTex->Map(MAP_READ, 0, 0);
		float3 result = *static_cast<float3*>(mapData.pData);
		resultTex->UnMap();

		/*radianceTex->Release();
		resultTex->Release();
*/
		return result;
	}

	void AtmosphereRendering::RecomputeSunRenderColor()
	{
		_sunRenderColor = ComputeSunRadianceAt(_sunDirection, _sunRadiance, 10.0f);
	}

	void AtmosphereRendering::InitOpticalDepthLUT()
	{
		int32_t lutNumHeights = 1024;
		int32_t lutNumAngles = 1024;

		TextureDesc texDesc;
		texDesc.width = lutNumHeights;
		texDesc.height = lutNumAngles;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		_opticalDepthLUT = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_opticalDepthLUT->SetDesc(texDesc);
		_opticalDepthLUT->Init();

		auto ps = Shader::FindOrCreate<InitOpticalDepthLUTPS>();

		ps->SetScalar("texSize", _opticalDepthLUT->GetTexSize());
		ps->SetScalar("earthRadius", _earthRadius);
		ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);
		ps->SetScalar("particleScaleHeight", float2(_particleScaleHeightR, _particleScaleHeightM));

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ _opticalDepthLUT->GetRenderTargetView(0, 0, 1) });

		/*float4 screenSize = 
			float4(
			static_cast<float>(texDesc.width),
			static_cast<float>(texDesc.height),
			1.0f / static_cast<float>(texDesc.width),
			1.0f / static_cast<float>(texDesc.height));*/
		/*_fx->VariableByName("texSize")->AsScalar()->SetValue(&_op);

		_fx->VariableByName("earthRadius")->AsScalar()->SetValue(&_earthRadius);
		_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&_atmosphereTopHeight);
		float2 particleScaleHeight = float2(_particleScaleHeightR, _particleScaleHeightM);
		_fx->VariableByName("particleScaleHeight")->AsScalar()->SetValue(&particleScaleHeight);*/

		/*auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ _opticalDepthLUT->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("InitOpticalDepthLUT"), 0, 0, texDesc.width, texDesc.height);*/
	}

	void AtmosphereRendering::InitInScatteringLUT()
	{
		_lutSize = int4(32, 128, 32, 8);
		int32_t lutSizeX = _lutSize.x();
		int32_t lutSizeY = _lutSize.y();
		int32_t lutSizeZ = _lutSize.z();
		int32_t lutSizeW = _lutSize.w();

		TextureDesc texDesc;
		texDesc.width = lutSizeX;
		texDesc.height = lutSizeY;
		texDesc.depth = lutSizeZ * lutSizeW;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		//texDesc.type = TEXTURE_3D;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		float2 wqTexelSize = 1.0f / float2(static_cast<float>(lutSizeZ), static_cast<float>(lutSizeW));

		//float4 lutSize = float4(static_cast<float>(lutSizeX), static_cast<float>(lutSizeY), static_cast<float>(lutSizeZ), static_cast<float>(lutSizeW));
		//_fx->VariableByName("lutSize")->AsScalar()->SetValue(&lutSize);

		float3 earthCenter = float3(0.0f, -_earthRadius, 0.0f);
		//_fx->VariableByName("earthCenter")->AsScalar()->SetValue(&earthCenter);
		//_fx->VariableByName("scatteringR")->AsScalar()->SetValue(&_scatteringR);
		//_fx->VariableByName("scatteringM")->AsScalar()->SetValue(&_scatteringM);
		//_fx->VariableByName("attenuationR")->AsScalar()->SetValue(&_attenuationR);
		//_fx->VariableByName("attenuationM")->AsScalar()->SetValue(&_attenuationM);
		//_fx->VariableByName("phaseG_M")->AsScalar()->SetValue(&_phaseG_M);

		//_fx->VariableByName("earthRadius")->AsScalar()->SetValue(&_earthRadius);
		//_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&_atmosphereTopHeight);
		float2 particleScaleHeight = float2(_particleScaleHeightR, _particleScaleHeightM);
		//_fx->VariableByName("particleScaleHeight")->AsScalar()->SetValue(&particleScaleHeight);

		//_fx->VariableByName("opticalDepthLUT")->AsShaderResource()->SetValue(_opticalDepthLUT->CreateTextureView());

		// Single Scattering
		auto singleScatteringR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		singleScatteringR->SetDesc(texDesc);
		singleScatteringR->Init();
		auto singleScatteringM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		singleScatteringM->SetDesc(texDesc);
		singleScatteringM->Init();
		{
			auto ps = Shader::FindOrCreate<PreComputeSingleScatteringPS>();
			for (int depth = 0; depth < texDesc.depth; ++depth)
			{
				ps->SetScalar("lutSize", float4((float)lutSizeX, (float)lutSizeY, (float)lutSizeZ, (float)lutSizeW));
				ps->SetScalar("earthCenter", earthCenter);
				ps->SetScalar("scatteringR", _scatteringR);
				ps->SetScalar("scatteringM", _scatteringM);
				ps->SetScalar("attenuationR", _attenuationR);
				ps->SetScalar("attenuationM", _attenuationM);
				ps->SetScalar("phaseG_M", _phaseG_M);

				ps->SetScalar("earthRadius", _earthRadius);
				ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);
				ps->SetScalar("particleScaleHeight", particleScaleHeight);

				float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x() + 0.5f * wqTexelSize.x();
				float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y() + 0.5f * wqTexelSize.y();
				float2 wqCoord = float2(wCoord, qCoord);
				ps->SetScalar("wqCoord", wqCoord);
				//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

				ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());

				ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
				ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

				ps->Flush();

				DrawQuad({
					singleScatteringR->GetRenderTargetView(0, depth, 1),
					singleScatteringM->GetRenderTargetView(0, depth, 1)
				});

				/*rc->SetRenderTargets({ singleScatteringR->CreateTextureView(0, 1, depth, 1), singleScatteringM->CreateTextureView(0, 1, depth, 1) }, 0);

				RenderQuad(_fx->TechniqueByName("PreComputeSingleScattering"), 0, 0, lutSizeX, lutSizeY);*/
			}
		}

		// Multi Scattering
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		auto radianceR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		radianceR->SetDesc(texDesc);
		radianceR->Init();
		auto radianceM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		radianceM->SetDesc(texDesc);
		radianceM->Init();
		auto scatteringOrderR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		scatteringOrderR->SetDesc(texDesc);
		scatteringOrderR->Init();
		auto scatteringOrderM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		scatteringOrderM->SetDesc(texDesc);
		scatteringOrderM->Init();
		auto scatteringAccumR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		scatteringAccumR->SetDesc(texDesc);
		scatteringAccumR->Init();
		auto scatteringAccumM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(TEXTURE_3D);
		scatteringAccumM->SetDesc(texDesc);
		scatteringAccumM->Init();

		for (int depth = 0; depth < texDesc.depth; ++depth)
		{
			rc->ClearRenderTarget(
			{ 
				scatteringAccumR->GetRenderTargetView(0, depth, 1),
				scatteringAccumM->GetRenderTargetView(0, depth, 1) 
			}, 0.0f);
		}

		int32_t numOrders = 5;
		for (int32_t orderIndex = 1; orderIndex < numOrders; ++orderIndex)
		{
			{
				auto ps = Shader::FindOrCreate<ComputeOutRadiancePS>();
				for (int depth = 0; depth < texDesc.depth; ++depth)
				{
					ps->SetScalar("lutSize", float4((float)lutSizeX, (float)lutSizeY, (float)lutSizeZ, (float)lutSizeW));
					ps->SetScalar("earthCenter", earthCenter);
					ps->SetScalar("scatteringR", _scatteringR);
					ps->SetScalar("scatteringM", _scatteringM);
					ps->SetScalar("attenuationR", _attenuationR);
					ps->SetScalar("attenuationM", _attenuationM);
					ps->SetScalar("phaseG_M", _phaseG_M);

					ps->SetScalar("earthRadius", _earthRadius);
					ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);
					ps->SetScalar("particleScaleHeight", particleScaleHeight);

					float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x() + 0.5f * wqTexelSize.x();
					float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y() + 0.5f * wqTexelSize.y();
					float2 wqCoord = float2(wCoord, qCoord);
					ps->SetScalar("wqCoord", wqCoord);
					//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);
					ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());

					if (orderIndex == 1)
					{
						ps->SetSRV("inScatteringLUTR", singleScatteringR->GetShaderResourceView());
						ps->SetSRV("inScatteringLUTM", singleScatteringM->GetShaderResourceView());

						/*_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(singleScatteringR->CreateTextureView(0, 1, 0, 0));
						_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(singleScatteringM->CreateTextureView(0, 1, 0, 0));*/
					}
					else
					{
						ps->SetSRV("inScatteringLUTR", scatteringOrderR->GetShaderResourceView());
						ps->SetSRV("inScatteringLUTM", scatteringOrderM->GetShaderResourceView());
						/*_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringAccumR->CreateTextureView(0, 1, 0, 0));
						_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringAccumM->CreateTextureView(0, 1, 0, 0));*/
					}

					ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
					ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

					ps->Flush();

					DrawQuad({
						radianceR->GetRenderTargetView(0, depth, 1),
						radianceM->GetRenderTargetView(0, depth, 1)
					});

					/*rc->SetRenderTargets({ radianceR->CreateTextureView(0, 1, depth, 1), radianceM->CreateTextureView(0, 1, depth, 1) }, 0);

					RenderQuad(_fx->TechniqueByName("ComputeOutRadiance"), 0, 0, lutSizeX, lutSizeY);*/
				}
			}

			{
				auto ps = Shader::FindOrCreate<PreComputeScatteringOrderPS>();
				for (int depth = 0; depth < texDesc.depth; ++depth)
				{
					ps->SetScalar("lutSize", float4((float)lutSizeX, (float)lutSizeY, (float)lutSizeZ, (float)lutSizeW));
					ps->SetScalar("earthCenter", earthCenter);
					ps->SetScalar("scatteringR", _scatteringR);
					ps->SetScalar("scatteringM", _scatteringM);
					ps->SetScalar("attenuationR", _attenuationR);
					ps->SetScalar("attenuationM", _attenuationM);
					ps->SetScalar("phaseG_M", _phaseG_M);

					ps->SetScalar("earthRadius", _earthRadius);
					ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);
					ps->SetScalar("particleScaleHeight", particleScaleHeight);

					float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x() + 0.5f * wqTexelSize.x();
					float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y() + 0.5f * wqTexelSize.y();
					float2 wqCoord = float2(wCoord, qCoord);
					ps->SetScalar("wqCoord", wqCoord);
					//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);
					ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());

					/*float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
					float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
					float2 wqCoord = float2(wCoord, qCoord);
					_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);*/

					ps->SetSRV("inScatteringLUTR", radianceR->GetShaderResourceView());
					ps->SetSRV("inScatteringLUTM", radianceM->GetShaderResourceView());

					ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
					ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

					ps->Flush();

					DrawQuad({
						scatteringOrderR->GetRenderTargetView(0, depth, 1),
						scatteringOrderM->GetRenderTargetView(0, depth, 1)
					});

					/*_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(radianceR->CreateTextureView(0, 1, 0, 0));
					_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(radianceM->CreateTextureView(0, 1, 0, 0));

					rc->SetRenderTargets({ scatteringOrderR->CreateTextureView(0, 1, depth, 1), scatteringOrderM->CreateTextureView(0, 1, depth, 1) }, 0);

					RenderQuad(_fx->TechniqueByName("PreComputeScatteringOrder"), 0, 0, lutSizeX, lutSizeY);*/
				}
			}

			{
				auto ps = Shader::FindOrCreate<AccumScatteringPS>();
				for (int depth = 0; depth < texDesc.depth; ++depth)
				{
					float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x() + 0.5f * wqTexelSize.x();
					float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y() + 0.5f * wqTexelSize.y();
					float2 wqCoord = float2(wCoord, qCoord);
					ps->SetScalar("wqCoord", wqCoord);
					//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

					//uint32_t depthSlice = static_cast<uint32_t>(depth);
					ps->SetScalar("depthSlice", depth);
					//_fx->VariableByName("depthSlice")->AsScalar()->SetValue(&depthSlice);

					ps->SetSRV("inScatteringLUTR", scatteringOrderR->GetShaderResourceView());
					ps->SetSRV("inScatteringLUTM", scatteringOrderM->GetShaderResourceView());

					ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
					ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

					ps->Flush();

					rc->SetBlendState(BlendStateTemplate<false, false,
						true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD,
						BLEND_PARAM_ONE, BLEND_PARAM_ZERO, BLEND_OP_ADD, COLOR_WRITE_ALL,
						true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD,
						BLEND_PARAM_ONE, BLEND_PARAM_ZERO, BLEND_OP_ADD, COLOR_WRITE_ALL>::Get());

					DrawQuad({
						scatteringAccumR->GetRenderTargetView(0, depth, 1),
						scatteringAccumM->GetRenderTargetView(0, depth, 1)
					});

					rc->SetBlendState(nullptr);

					/*_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringOrderR->CreateTextureView(0, 1, 0, 0));
					_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringOrderM->CreateTextureView(0, 1, 0, 0));

					rc->SetRenderTargets({ scatteringAccumR->CreateTextureView(0, 1, depth, 1), scatteringAccumM->CreateTextureView(0, 1, depth, 1) }, 0);

					RenderQuad(_fx->TechniqueByName("AddScattering"), 0, 0, lutSizeX, lutSizeY);*/
				}
			}
		}

		{
			auto ps = Shader::FindOrCreate<AccumScatteringPS>();
			for (int depth = 0; depth < texDesc.depth; ++depth)
			{
				float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x() + 0.5f * wqTexelSize.x();
				float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y() + 0.5f * wqTexelSize.y();
				float2 wqCoord = float2(wCoord, qCoord);
				ps->SetScalar("wqCoord", wqCoord);
				//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

				//uint32_t depthSlice = static_cast<uint32_t>(depth);
				ps->SetScalar("depthSlice", depth);
				//_fx->VariableByName("depthSlice")->AsScalar()->SetValue(&depthSlice);

				ps->SetSRV("inScatteringLUTR", scatteringAccumR->GetShaderResourceView());
				ps->SetSRV("inScatteringLUTM", scatteringAccumM->GetShaderResourceView());

				ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
				ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

				ps->Flush();

				rc->SetBlendState(BlendStateTemplate<false, false,
					true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD,
					BLEND_PARAM_ONE, BLEND_PARAM_ZERO, BLEND_OP_ADD, 0xff,
					true, BLEND_PARAM_ONE, BLEND_PARAM_ONE, BLEND_OP_ADD,
					BLEND_PARAM_ONE, BLEND_PARAM_ZERO, BLEND_OP_ADD, 0xff>::Get());

				DrawQuad({
					singleScatteringR->GetRenderTargetView(0, depth, 1),
					singleScatteringM->GetRenderTargetView(0, depth, 1)
				});

				rc->SetBlendState(nullptr);

				/*_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringOrderR->CreateTextureView(0, 1, 0, 0));
				_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringOrderM->CreateTextureView(0, 1, 0, 0));

				rc->SetRenderTargets({ scatteringAccumR->CreateTextureView(0, 1, depth, 1), scatteringAccumM->CreateTextureView(0, 1, depth, 1) }, 0);

				RenderQuad(_fx->TechniqueByName("AddScattering"), 0, 0, lutSizeX, lutSizeY);*/
			}
		}

		//{
		//	auto ps = Shader::FindOrCreate<AccumSingleScatteringPS>();
		//	for (int depth = 0; depth < texDesc.depth; ++depth)
		//	{
		//		ps->SetScalar("lutSize", float4((float)lutSizeX, (float)lutSizeY, (float)lutSizeZ, (float)lutSizeW));
		//		ps->SetScalar("earthCenter", earthCenter);
		//		ps->SetScalar("scatteringR", _scatteringR);
		//		ps->SetScalar("scatteringM", _scatteringM);
		//		ps->SetScalar("attenuationR", _attenuationR);
		//		ps->SetScalar("attenuationM", _attenuationM);
		//		ps->SetScalar("phaseG_M", _phaseG_M);

		//		ps->SetScalar("earthRadius", _earthRadius);
		//		ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);
		//		ps->SetScalar("particleScaleHeight", particleScaleHeight);

		//		float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
		//		float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
		//		float2 wqCoord = float2(wCoord, qCoord);
		//		ps->SetScalar("wqCoord", wqCoord);
		//		//_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

		//		//uint32_t depthSlice = static_cast<uint32_t>(depth);
		//		ps->SetScalar("depthSlice", depth);
		//		//_fx->VariableByName("depthSlice")->AsScalar()->SetValue(&depthSlice);

		//		ps->SetSRV("inScatteringLUTR", scatteringAccumR->GetShaderResourceView());
		//		ps->SetSRV("inScatteringLUTM", scatteringAccumM->GetShaderResourceView());

		//		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		//		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		//		ps->Flush();

		//		DrawQuad({
		//			singleScatteringR->GetRenderTargetView(0, depth, 1),
		//			singleScatteringM->GetRenderTargetView(0, depth, 1),
		//		});

		//		//_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringAccumR->CreateTextureView(0, 1, 0, 0));
		//		//_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringAccumM->CreateTextureView(0, 1, 0, 0));

		//		//rc->SetRenderTargets({ singleScatteringR->CreateTextureView(0, 1, depth, 1) }, 0);

		//		//RenderQuad(_fx->TechniqueByName("AddSingleScattering"), 0, 0, lutSizeX, lutSizeY);
		//	}
		//}

		_inScatteringLUTR = singleScatteringR;
		_inScatteringLUTM = singleScatteringM;
	}

	PooledTextureRef AtmosphereRendering::InitSampleLines(
		const float4 & viewSize,
		const float2 & lightClipPos)
	{
		TextureDesc texDesc;
		texDesc.width = _numSampleLines;
		texDesc.height = texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		auto resultTexRef = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		std::map<String, String> macros;
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);

		auto ps = Shader::FindOrCreate<InitSampleLinesPS>(macros);

		ps->SetScalar("texSize", viewSize);
		ps->SetScalar("lightClipPos", lightClipPos);

		uint32_t bLightInScreen = 
				std::abs(lightClipPos.x()) <= 1.0f - viewSize.z()
			&&	std::abs(lightClipPos.y()) <= 1.0f - viewSize.w();
		ps->SetScalar("bLightInScreen", bLightInScreen);

		ps->Flush();

		DrawQuad({ resultTexRef->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) });

		return resultTexRef;
	}

	void AtmosphereRendering::InitSampleCoordsTex(
		const float4 & viewSize,
		const Ptr<Texture> & sampleLinesTex,
		const Ptr<Texture> & sceneLinearDepthTex,
		PooledTextureRef & outSampleCoordTex,
		PooledTextureRef & outSampleDepthTex,
		PooledTextureRef & outSampleMaskDS)
	{
		//Init textures
		TextureDesc texDesc;
		texDesc.width = _maxSamplesPerLine;
		texDesc.height = _numSampleLines;
		texDesc.depth = 1;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		texDesc.format = RENDER_FORMAT_R16G16_FLOAT;
		outSampleCoordTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		outSampleDepthTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		texDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		texDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		outSampleMaskDS = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		std::map<String, String> macros;
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);
		if(!sceneLinearDepthTex)
			macros["NO_DEPTH"] = "";

		auto ps = Shader::FindOrCreate<InitSampleCoordsPS>(macros);

		ps->SetScalar("texSize", viewSize);

		ps->SetSRV("sampleLinesTex", sampleLinesTex->GetShaderResourceView());
		if(sceneLinearDepthTex)
			ps->SetSRV("sceneLinearDepthTex", sceneLinearDepthTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->ClearRenderTarget({
			outSampleCoordTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
			outSampleDepthTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1)
		}, -1e+30f);

		rc->ClearDepthStencil(outSampleMaskDS->Get()->Cast<Texture>()->GetDepthStencilView(0, 0, 1), 1.0f, 0);

		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_INCR,
			COMPARISON_ALWAYS>::Get());

		DrawQuad({
			outSampleCoordTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
			outSampleDepthTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) },
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			outSampleMaskDS->Get()->Cast<Texture>()->GetDepthStencilView(0, 0, 1));

		rc->SetDepthStencilState(nullptr);
	}

	PooledTextureRef AtmosphereRendering::RefineSamples(
		const float4 & viewSize,
		float depthBreakThreshold,
		const Ptr<Texture> & sampleCoordsTex,
		const Ptr<Texture> & sampleDepthTex)
	{
		TextureDesc texDesc;
		texDesc.width = _maxSamplesPerLine;
		texDesc.height = _numSampleLines;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16_UINT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		auto interpolationSourceTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });

		std::map<String, String> macros;
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);
		macros["INITIAL_SAMPLE_STEP"] = std::to_string(_initalSampleStep);

		auto cs = Shader::FindOrCreate<RefineSamplesCS>(macros);
		
		cs->SetScalar("depthBreakThreshold", depthBreakThreshold);
		cs->SetScalar("texSize", viewSize);

		cs->SetUAV("interpolationSourceTex", interpolationSourceTex->Get()->Cast<Texture>()->GetUnorderedAccessView(0, 0, 1));
		cs->SetSRV("sampleCoordsTex", sampleCoordsTex->GetShaderResourceView());
		cs->SetSRV("sampleDepthTex", sampleDepthTex->GetShaderResourceView());

		cs->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->Compute(_maxSamplesPerLine / std::max<int32_t>(_initalSampleStep, 32), _numSampleLines, 1);

		rc->ResetShader(SHADER_CS);

		return interpolationSourceTex;
	}

	void AtmosphereRendering::MarkRayMarchingSamples(
		const Ptr<Texture> & interpolationSourceTex, 
		const Ptr<Texture> & sampleMaskDS)
	{
		auto ps = Shader::FindOrCreate<MarkRayMarchingSamplesPS>();

		ps->SetSRV("interpolationSourceTex", interpolationSourceTex->GetShaderResourceView());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_INCR,
			COMPARISON_EQUAL>::Get(), 1);

		DrawQuad({},
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			sampleMaskDS->GetDepthStencilView(0, 0, 1));

		rc->SetDepthStencilState(nullptr);
	}

	void AtmosphereRendering::DoRayMarching(
		const Ptr<RenderView> & view,
		const Ptr<Texture> & sampleCoordsTex,
		const Ptr<Texture> & depthTex,
		const Ptr<Texture> & sampleMaskDS,
		PooledTextureRef & outLightAccumTex,
		PooledTextureRef & outAttenuationTex)
	{
		TextureDesc texDesc;
		if (bEpipolarSampling)
		{
			texDesc.width = _maxSamplesPerLine;
			texDesc.height = _numSampleLines;
		}
		else
		{
			texDesc.width = (int32_t)view->GetViewParams().viewSize.x();
			texDesc.height = (int32_t)view->GetViewParams().viewSize.y();
		}
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;

		auto lightAccumTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		auto attenuationTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(lightAccumTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(attenuationTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);

		std::map<String, String> macros;
		if (bEpipolarSampling)
			macros["EPIPOLAR_SAMPLING"] = "";
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);
		if (!depthTex)
			macros["NO_DEPTH"] = "";

		auto ps = Shader::FindOrCreate<RayMarchingPS>(macros);

		view->BindShaderParams(ps);

		ps->SetScalar("lightRadiance", _sunRadiance);
		ps->SetScalar("lightDirection", _sunDirection);

		ps->SetScalar("lutSize", float4((float)_lutSize.x(), (float)_lutSize.y(), (float)_lutSize.z(), (float)_lutSize.w()));

		ps->SetScalar("earthCenter", float3(0.0f, -_earthRadius, 0.0f));
		ps->SetScalar("earthRadius", _earthRadius);
		ps->SetScalar("atmosphereTopHeight", _atmosphereTopHeight);

		ps->SetScalar("particleScaleHeight", float2(_particleScaleHeightR, _particleScaleHeightM));
		ps->SetScalar("scatteringR", _scatteringR);
		ps->SetScalar("scatteringM", _scatteringM);
		ps->SetScalar("attenuationR", _attenuationR);
		ps->SetScalar("attenuationM", _attenuationM);
		ps->SetScalar("phaseG_M", _phaseG_M);

		if (bEpipolarSampling)
		{
			ps->SetSRV("sampleCoordsTex", sampleCoordsTex->GetShaderResourceView());
			ps->SetSRV("sampleDepthTex", depthTex->GetShaderResourceView());
		}
		else
		{
			if(depthTex)
				ps->SetSRV("sceneLinearDepthTex", depthTex->GetShaderResourceView());
		}
		ps->SetSRV("opticalDepthLUT", _opticalDepthLUT->GetShaderResourceView());
		ps->SetSRV("inScatteringLUTR", _inScatteringLUTR->GetShaderResourceView());
		ps->SetSRV("inScatteringLUTM", _inScatteringLUTM->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		if (bEpipolarSampling)
		{
			auto rc = Global::GetRenderEngine()->GetRenderContext();
			rc->SetDepthStencilState(
				DepthStencilStateTemplate<
				false,
				DEPTH_WRITE_ZERO,
				COMPARISON_LESS,
				true, 0xff, 0xff,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				STENCIL_OP_KEEP,
				COMPARISON_EQUAL>::Get(), 2);

			DrawQuad({
				lightAccumTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
				attenuationTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) },
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
				sampleMaskDS->GetDepthStencilView(0, 0, 1));

			rc->SetDepthStencilState(nullptr);
		}
		else
		{
			DrawQuad({
				lightAccumTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
				attenuationTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) }
			);
		}

		outLightAccumTex = lightAccumTex;
		outAttenuationTex = attenuationTex;
	}

	void AtmosphereRendering::InterpolateRestSamples(
		const float4 & viewSize,
		const Ptr<Texture> & interpolationSourceTex,
		const Ptr<Texture> & sampleDepthTex,
		const Ptr<Texture> & lightAccumTex,
		const Ptr<Texture> & attenuationTex,
		const Ptr<Texture> & sampleMaskDS,
		PooledTextureRef & outLightAccumTex,
		PooledTextureRef & outAttenuationTex)
	{
		TextureDesc texDesc = lightAccumTex->GetDesc();

		outLightAccumTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		outAttenuationTex = TexturePool::Instance().FindFree({ TEXTURE_2D, texDesc });
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(outLightAccumTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);
		Global::GetRenderEngine()->GetRenderContext()->ClearRenderTarget(outAttenuationTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1), 0.0f);

		std::map<String, String> macros;
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);

		auto ps = Shader::FindOrCreate<InterpolateRestSamplesPS>(macros);

		ps->SetScalar("texSize", viewSize);

		ps->SetSRV("interpolationSourceTex", interpolationSourceTex->GetShaderResourceView());
		ps->SetSRV("sampleDepthTex", sampleDepthTex->GetShaderResourceView());
		ps->SetSRV("lightAccumTex", lightAccumTex->GetShaderResourceView());
		ps->SetSRV("attenuationTex", attenuationTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			COMPARISON_NOT_EQUAL>::Get(), 0);

		DrawQuad({
			outLightAccumTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1),
			outAttenuationTex->Get()->Cast<Texture>()->GetRenderTargetView(0, 0, 1) },
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			sampleMaskDS->GetDepthStencilView(0, 0, 1));

		rc->SetDepthStencilState(nullptr);
	}

	void AtmosphereRendering::UnWarpSamples(
		const float2 & lightClipPos,
		const Ptr<Texture> & lightAccumTex,
		const Ptr<Texture> & attenuationTex,
		const Ptr<Texture> & sampleLinesTex,
		const Ptr<Texture> & sampleDepthTex,
		const Ptr<Texture> & sceneTex,
		const Ptr<Texture> & sceneLinearDepthTex,
		const Ptr<RenderTargetView> & target)
	{
		std::map<String, String> macros;
		macros["NUM_SAMPLELINES"] = std::to_string(_numSampleLines);
		macros["MAX_SAMPLES_PERLINE"] = std::to_string(_maxSamplesPerLine);
		if (!sceneLinearDepthTex)
			macros["NO_DEPTH"] = "";

		auto ps = Shader::FindOrCreate<UnWarpSamplesPS>(macros);

		ps->SetScalar("lightClipPos", lightClipPos);
		ps->SetScalar("texSize", sceneTex->GetTexSize());

		ps->SetSRV("lightAccumTex", lightAccumTex->GetShaderResourceView());
		ps->SetSRV("attenuationTex", attenuationTex->GetShaderResourceView());
		ps->SetSRV("sampleLinesTex", sampleLinesTex->GetShaderResourceView());
		ps->SetSRV("sampleDepthTex", sampleDepthTex->GetShaderResourceView());
		ps->SetSRV("sceneTex", sceneTex->GetShaderResourceView());
		if(sceneLinearDepthTex)
			ps->SetSRV("sceneLinearDepthTex", sceneLinearDepthTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());
		ps->SetSampler("linearSampler", SamplerTemplate<>::Get());

		ps->Flush();

		DrawQuad({ target });
	}

	void AtmosphereRendering::AccumRayMarching(
		const Ptr<Texture> & lightAccumTex,
		const Ptr<Texture> & attenuationTex,
		const Ptr<RenderTargetView> & target)
	{
		auto ps = Shader::FindOrCreate<AccumRayMarchingPS>();

		ps->SetSRV("lightAccumTex", lightAccumTex->GetShaderResourceView());
		ps->SetSRV("attenuationTex", attenuationTex->GetShaderResourceView());

		ps->SetSampler("pointSampler", SamplerTemplate<FILTER_MIN_MAG_MIP_POINT>::Get());

		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_ONE, BLEND_PARAM_SRC1_COLOR, BLEND_OP_ADD>::Get());

		DrawQuad({ target });

		rc->SetBlendState(nullptr);
	}

	void AtmosphereRendering::DoRenderSun(
		const float2 & lightClipPos,
		const Ptr<RenderView> & view,
		const Ptr<RenderTargetView> & target,
		const Ptr<DepthStencilView> & sceneDepthTex)
	{
		auto vs = Shader::FindOrCreate<RenderSunVS>();
		auto ps = Shader::FindOrCreate<RenderSunPS>();

		view->BindShaderParams(vs);
		view->BindShaderParams(ps);

		float sunAngularRadius = _sunRenderRadius / 60.0f * (PI2 / 360.0f);
		float sunRadius = std::tan(sunAngularRadius);
		vs->SetScalar("sunRadius", sunRadius);
		ps->SetScalar("sunRadius", sunRadius);

		vs->SetScalar("lightClipPos", lightClipPos);
		ps->SetScalar("lightClipPos", lightClipPos);

		ps->SetScalar("sunRadiance", _sunRenderColor);

		vs->Flush();
		ps->Flush();

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetViewport(GetTextureQuadViewport(target->GetResource()->Cast<Texture>()));

		rc->SetRenderTargets({ target });
		rc->SetDepthStencil(sceneDepthTex);
		rc->SetDepthStencilState(
			DepthStencilStateTemplate<
			false,
			DEPTH_WRITE_ZERO,
			COMPARISON_LESS,
			true, 0xff, 0xff,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			STENCIL_OP_KEEP,
			COMPARISON_EQUAL>::Get(), 0);

		rc->SetBlendState(BlendStateTemplate<false, false, true, BLEND_PARAM_SRC_ALPHA, BLEND_PARAM_ONE, BLEND_OP_ADD, BLEND_PARAM_ZERO>::Get());

		rc->SetVertexBuffer({});
		rc->SetIndexBuffer(nullptr);
		rc->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		rc->DrawVertices(4, 0);

		rc->SetDepthStencil(nullptr);
		rc->SetBlendState(nullptr);
	}
}
