#include "ToyGE\RenderEngine\Effects\AtmosphereRendering.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderContext.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\RenderEngine\RenderEffect.h"
#include "ToyGE\RenderEngine\RenderSharedEnviroment.h"
#include "ToyGE\RenderEngine\RenderView.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\RenderEngine\LightComponent.h"
#include "ToyGE\RenderEngine\Camera.h"
#include "ToyGE\RenderEngine\DeferredRenderFramework.h"
#include "ToyGE\RenderEngine\ShadowTechnique.h"
#include "ToyGE\RenderEngine\RenderInput.h"

namespace ToyGE
{
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
		std::vector<MacroDesc> macros;
		macros.push_back({ "NUM_SAMPLELINES", std::to_string(_numSampleLines) });
		macros.push_back({ "MAX_SAMPLES_PERLINE", std::to_string(_maxSamplesPerLine) });

		_fx = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"AtmosphereRendering.xml");
		_fx->SetExtraMacros(macros);

		_refineFX = Global::GetResourceManager(RESOURCE_EFFECT)->As<EffectManager>()->AcquireResource(L"EpipolarSamplingRefineCS.xml");
		_refineFX->SetExtraMacros(macros);

		_scatteringR = float3(5.8f, 13.5f, 33.1f) * 1e-6f;
		_scatteringM = float3(2.0f) * 1e-5f;
		_attenuationR = _scatteringR;
		_attenuationM = 1.1f * _scatteringM;
	}

	void AtmosphereRendering::Render(const Ptr<RenderSharedEnviroment> & sharedEnviroment)
	{
		if (!_opticalDepthLUT)
			InitOpticalDepthLUT();
		if (!_inScatteringLUTR)
			InitInScatteringLUT();

		//auto targetTex = std::static_pointer_cast<Texture>(sharedEnviroment->GetView()->GetRenderTarget().resource);

		float2 targetSize = float2(static_cast<float>(sharedEnviroment->GetView()->GetRenderTarget()->Desc().width), static_cast<float>(sharedEnviroment->GetView()->GetRenderTarget()->Desc().height));

		auto linearDepthTex = sharedEnviroment->ParamByName(CommonRenderShareName::LinearDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		auto & cameraPos = sharedEnviroment->GetView()->GetCamera()->Pos();
		float sunDist = 1e+10;
		auto sunPosW = XMFLOAT3(
			cameraPos.x - _sunDirection.x * sunDist,
			cameraPos.y - _sunDirection.y * sunDist,
			cameraPos.z - _sunDirection.z * sunDist); //*reinterpret_cast<XMFLOAT3*>(&_sun->GetPosOffsetVec());
		auto sunPosWXM = XMLoadFloat3(&sunPosW);
		auto viewXM = XMLoadFloat4x4(&sharedEnviroment->GetView()->GetCamera()->ViewMatrix());
		auto sunPosVXM = XMVector3TransformCoord(sunPosWXM, viewXM);
		sunPosVXM = XMVectorSetW(sunPosVXM, 1.0f);
		auto projXM = XMLoadFloat4x4(&sharedEnviroment->GetView()->GetCamera()->ProjMatrix());
		auto sunPosHXM = XMVector4Transform(sunPosVXM, projXM);
		float4 sunPosH;
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&sunPosH), sunPosHXM);
		sunPosH.x /= sunPosH.w;
		sunPosH.y /= sunPosH.w;
		sunPosH.z /= sunPosH.w;

		auto sampleLinesTex = InitSampleLines(targetSize, sharedEnviroment->GetView()->GetCamera(), sunPosH.v(VEC_X, VEC_Y));

		Ptr<Texture> sampleCoordsTex, cameraDepthTex, depthStencilTex;
		InitSampleCoordsTex(sampleLinesTex, linearDepthTex, sampleCoordsTex, cameraDepthTex, depthStencilTex);

		auto interplationSourceTex = RefineSamples(sampleCoordsTex, cameraDepthTex, sharedEnviroment->GetView()->GetCamera(), sunPosH.v(VEC_X, VEC_Y));

		MarkRayMarchingSamples(interplationSourceTex, depthStencilTex);

		Ptr<Texture> lightAccumTex, attenuationTex;
		DoRayMarching(sampleCoordsTex, cameraDepthTex, depthStencilTex, sharedEnviroment->GetView()->GetCamera(), lightAccumTex, attenuationTex);

		Ptr<Texture> lightAccumInterpTex, attenuationInterpTex;
		InterpolateRestSamples(interplationSourceTex, cameraDepthTex, lightAccumTex, attenuationTex, lightAccumInterpTex, attenuationInterpTex);

		//auto sceneTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		//targetTex->CopyTo(sceneTex, 0, 0, 0, 0, 0, 0, 0);
		//auto tmpTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(targetTex->Desc());
		UnWarpSamples(lightAccumInterpTex, attenuationInterpTex, sharedEnviroment->GetView()->GetRenderResult(), sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView());

		auto rawDepthStencil = sharedEnviroment->ParamByName(CommonRenderShareName::RawDepth())->As<SharedParam<Ptr<Texture>>>()->GetValue();

		if (XMVectorGetZ(sunPosVXM) < 0.0f)
			sunPosH.x = sunPosH.y = -100.0f;
		RenderSun(sunPosH, sharedEnviroment->GetView()->GetCamera(), sharedEnviroment->GetView()->GetRenderTarget()->CreateTextureView(), rawDepthStencil);

		sharedEnviroment->GetView()->FlipRenderTarget();

		sampleLinesTex->Release();
		sampleCoordsTex->Release();
		cameraDepthTex->Release();
		depthStencilTex->Release();
		interplationSourceTex->Release();
		lightAccumTex->Release();
		attenuationTex->Release();
		lightAccumInterpTex->Release();
		attenuationInterpTex->Release();
		//sceneTex->Release();
	}

	XMFLOAT3 AtmosphereRendering::ComputeSunRadianceAt(const XMFLOAT3 & sunDir, const XMFLOAT3 & sunRadiance, float height)
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
		texDesc.type = TEXTURE_2D;

		auto radianceTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		_fx->VariableByName("opticalDepthLUT")->AsShaderResource()->SetValue(_opticalDepthLUT->CreateTextureView());

		_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&sunRadiance);
		_fx->VariableByName("lightDirection")->AsScalar()->SetValue(&sunDir);
		_fx->VariableByName("computeSunRadianceHeight")->AsScalar()->SetValue(&height);

		_fx->VariableByName("attenuationR")->AsScalar()->SetValue(&_attenuationR);
		_fx->VariableByName("attenuationM")->AsScalar()->SetValue(&_attenuationM);

		_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&_atmosphereTopHeight);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ radianceTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("ComputeSunRadiance"), 0, 0, 1, 1);

		texDesc.bindFlag = 0;
		texDesc.cpuAccess = CPU_ACCESS_READ;
		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		radianceTex->CopyTo(resultTex, 0, 0, 0, 0, 0, 0, 0);

		auto mapData = resultTex->Map(MAP_READ, 0, 0);
		XMFLOAT3 result = *static_cast<XMFLOAT3*>(mapData.pData);
		resultTex->UnMap();

		radianceTex->Release();
		resultTex->Release();

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
		texDesc.type = TEXTURE_2D;

		_opticalDepthLUT = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		float4 screenSize = 
			float4(
			static_cast<float>(texDesc.width),
			static_cast<float>(texDesc.height),
			1.0f / static_cast<float>(texDesc.width),
			1.0f / static_cast<float>(texDesc.height));
		_fx->VariableByName("screenSize")->AsScalar()->SetValue(&screenSize);

		_fx->VariableByName("earthRadius")->AsScalar()->SetValue(&_earthRadius);
		_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&_atmosphereTopHeight);
		float2 particleScaleHeight = float2(_particleScaleHeightR, _particleScaleHeightM);
		_fx->VariableByName("particleScaleHeight")->AsScalar()->SetValue(&particleScaleHeight);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ _opticalDepthLUT->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("InitOpticalDepthLUT"), 0, 0, texDesc.width, texDesc.height);
	}

	void AtmosphereRendering::InitInScatteringLUT()
	{
		int32_t lutSizeX = 32;
		int32_t lutSizeY = 128;
		int32_t lutSizeZ = 32;
		int32_t lutSizeW = 8;

		TextureDesc texDesc;
		texDesc.width = lutSizeX;
		texDesc.height = lutSizeY;
		texDesc.depth = lutSizeZ * lutSizeW;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R16G16B16A16_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_3D;

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		float2 wqTexelSize = 1.0f / float2(static_cast<float>(lutSizeZ), static_cast<float>(lutSizeW));

		float4 lutSize = float4(static_cast<float>(lutSizeX), static_cast<float>(lutSizeY), static_cast<float>(lutSizeZ), static_cast<float>(lutSizeW));
		_fx->VariableByName("lutSize")->AsScalar()->SetValue(&lutSize);

		float3 earthCenter = float3(0.0f, -_earthRadius, 0.0f);
		_fx->VariableByName("earthCenter")->AsScalar()->SetValue(&earthCenter);
		_fx->VariableByName("scatteringR")->AsScalar()->SetValue(&_scatteringR);
		_fx->VariableByName("scatteringM")->AsScalar()->SetValue(&_scatteringM);
		_fx->VariableByName("attenuationR")->AsScalar()->SetValue(&_attenuationR);
		_fx->VariableByName("attenuationM")->AsScalar()->SetValue(&_attenuationM);
		_fx->VariableByName("phaseG_M")->AsScalar()->SetValue(&_phaseG_M);

		_fx->VariableByName("earthRadius")->AsScalar()->SetValue(&_earthRadius);
		_fx->VariableByName("atmosphereTopHeight")->AsScalar()->SetValue(&_atmosphereTopHeight);
		float2 particleScaleHeight = float2(_particleScaleHeightR, _particleScaleHeightM);
		_fx->VariableByName("particleScaleHeight")->AsScalar()->SetValue(&particleScaleHeight);

		_fx->VariableByName("opticalDepthLUT")->AsShaderResource()->SetValue(_opticalDepthLUT->CreateTextureView());

		//Single Scattering
		auto singleScatteringR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto singleScatteringM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		for (int depth = 0; depth < texDesc.depth; ++depth)
		{
			float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
			float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
			float2 wqCoord = float2(wCoord, qCoord);
			_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

			rc->SetRenderTargets({ singleScatteringR->CreateTextureView(0, 1, depth, 1), singleScatteringM->CreateTextureView(0, 1, depth, 1) }, 0);

			RenderQuad(_fx->TechniqueByName("PreComputeSingleScattering"), 0, 0, lutSizeX, lutSizeY);
		}

		//Multi Scattering
		texDesc.format = RENDER_FORMAT_R32G32B32A32_FLOAT;
		auto radianceR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto radianceM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto scatteringOrderR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto scatteringOrderM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto scatteringAccumR = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);
		auto scatteringAccumM = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(texDesc);

		for (int depth = 0; depth < texDesc.depth; ++depth)
		{
			rc->ClearRenderTargets(
			{ 
				scatteringAccumR->CreateTextureView(0, 1, depth, 1),
				scatteringAccumM->CreateTextureView(0, 1, depth, 1) }, 0.0f);
		}

		int32_t numOrders = 4;
		for (int32_t orderIndex = 1; orderIndex < numOrders; ++orderIndex)
		{
			for (int depth = 0; depth < texDesc.depth; ++depth)
			{
				float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
				float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
				float2 wqCoord = float2(wCoord, qCoord);
				_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

				if (orderIndex == 1)
				{
					_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(singleScatteringR->CreateTextureView(0, 1, 0, 0));
					_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(singleScatteringM->CreateTextureView(0, 1, 0, 0));
				}
				else
				{
					_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringAccumR->CreateTextureView(0, 1, 0, 0));
					_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringAccumM->CreateTextureView(0, 1, 0, 0));
				}

				rc->SetRenderTargets({ radianceR->CreateTextureView(0, 1, depth, 1), radianceM->CreateTextureView(0, 1, depth, 1) }, 0);

				RenderQuad(_fx->TechniqueByName("ComputeOutRadiance"), 0, 0, lutSizeX, lutSizeY);
			}

			for (int depth = 0; depth < texDesc.depth; ++depth)
			{
				float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
				float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
				float2 wqCoord = float2(wCoord, qCoord);
				_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

				_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(radianceR->CreateTextureView(0, 1, 0, 0));
				_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(radianceM->CreateTextureView(0, 1, 0, 0));

				rc->SetRenderTargets({ scatteringOrderR->CreateTextureView(0, 1, depth, 1), scatteringOrderM->CreateTextureView(0, 1, depth, 1) }, 0);

				RenderQuad(_fx->TechniqueByName("PreComputeScatteringOrder"), 0, 0, lutSizeX, lutSizeY);
			}

			for (int depth = 0; depth < texDesc.depth; ++depth)
			{
				float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
				float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
				float2 wqCoord = float2(wCoord, qCoord);
				_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

				uint32_t depthSlice = static_cast<uint32_t>(depth);
				_fx->VariableByName("depthSlice")->AsScalar()->SetValue(&depthSlice);

				_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringOrderR->CreateTextureView(0, 1, 0, 0));
				_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringOrderM->CreateTextureView(0, 1, 0, 0));

				rc->SetRenderTargets({ scatteringAccumR->CreateTextureView(0, 1, depth, 1), scatteringAccumM->CreateTextureView(0, 1, depth, 1) }, 0);

				RenderQuad(_fx->TechniqueByName("AddScattering"), 0, 0, lutSizeX, lutSizeY);
			}
		}

		for (int depth = 0; depth < texDesc.depth; ++depth)
		{
			float wCoord = static_cast<float>(depth % lutSizeZ) * wqTexelSize.x + 0.5f * wqTexelSize.x;
			float qCoord = static_cast<float>(depth / lutSizeZ) * wqTexelSize.y + 0.5f * wqTexelSize.y;
			float2 wqCoord = float2(wCoord, qCoord);
			_fx->VariableByName("wqCoord")->AsScalar()->SetValue(&wqCoord);

			uint32_t depthSlice = static_cast<uint32_t>(depth);
			_fx->VariableByName("depthSlice")->AsScalar()->SetValue(&depthSlice);

			_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(scatteringAccumR->CreateTextureView(0, 1, 0, 0));
			_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(scatteringAccumM->CreateTextureView(0, 1, 0, 0));

			rc->SetRenderTargets({ singleScatteringR->CreateTextureView(0, 1, depth, 1) }, 0);

			RenderQuad(_fx->TechniqueByName("AddSingleScattering"), 0, 0, lutSizeX, lutSizeY);
		}

		_inScatteringLUTR = singleScatteringR;
		_inScatteringLUTM = singleScatteringM;
	}

	Ptr<Texture> AtmosphereRendering::InitSampleLines(const float2 & renderTargetSize, const Ptr<Camera> & camera, const float2 & lightPosH)
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
		texDesc.type = TEXTURE_2D;

		auto resultTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		float4 screenSize = float4(renderTargetSize.x, renderTargetSize.y, 1.0f / renderTargetSize.x, 1.0f / renderTargetSize.y);
		_fx->VariableByName("screenSize")->AsScalar()->SetValue(&screenSize);

		
		_fx->VariableByName("lightPosH")->AsScalar()->SetValue(&lightPosH);

		bool bLightInScreen = 
				std::abs(lightPosH.x) <= 1.0f - screenSize.z
			&&	std::abs(lightPosH.y) <= 1.0f - screenSize.w;
		uint32_t bLightInScreenU = bLightInScreen;
		_fx->VariableByName("bLightInScreen")->AsScalar()->SetValue(&bLightInScreenU);

		rc->SetRenderTargets({ resultTex->CreateTextureView() }, 0);
		RenderQuad(_fx->TechniqueByName("InitSampleLines"), 0, 0, texDesc.width, texDesc.height);

		return resultTex;
	}

	void AtmosphereRendering::InitSampleCoordsTex(
		const Ptr<Texture> & sampleLinesTex,
		const Ptr<Texture> & linearDepthTex,
		Ptr<Texture> & outSampleCoordTex,
		Ptr<Texture> & outCameraDepthTex,
		Ptr<Texture> & outDepthStencilTex)
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
		texDesc.type = TEXTURE_2D;

		texDesc.format = RENDER_FORMAT_R16G16_FLOAT;
		outSampleCoordTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.format = RENDER_FORMAT_R32_FLOAT;
		outCameraDepthTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		texDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		texDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		outDepthStencilTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("sampleLinesTex")->AsShaderResource()->SetValue(sampleLinesTex->CreateTextureView());
		_fx->VariableByName("linearDepthTex")->AsShaderResource()->SetValue(linearDepthTex->CreateTextureView());

		rc->SetRenderTargets({ outSampleCoordTex->CreateTextureView(), outCameraDepthTex->CreateTextureView() }, 0);
		float invalidValue = -1e+30f;
		rc->ClearRenderTargets({ invalidValue, invalidValue, invalidValue, invalidValue });

		rc->ClearDepthStencil({ outDepthStencilTex->CreateTextureView() }, 1.0f, 0);

		float4 screenSize = float4(1280.0f, 720.0f, 1.0f / 1280.0f, 1.0f / 720.0f);
		_fx->VariableByName("screenSize")->AsScalar()->SetValue(&screenSize);

		RenderQuad(
			_fx->TechniqueByName("InitSampleCoords"),
			0, 0,
			_maxSamplesPerLine, _numSampleLines,
			outDepthStencilTex->CreateTextureView());
	}

	Ptr<Texture> AtmosphereRendering::RefineSamples(
		const Ptr<Texture> & sampleCoordsTex,
		const Ptr<Texture> & cameraDepthTex,
		const Ptr<Camera> & camera,
		const float2 & lightPosH)
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
		texDesc.type = TEXTURE_2D;

		auto interpolationSourceTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		//_refineFX->VariableByName("lightPosH")->AsScalar()->SetValue(&lightPosH);
		//uint32_t sampleDensityFactor = 2;
		//_refineFX->VariableByName("sampleDensityFactor")->AsScalar()->SetValue(&sampleDensityFactor);

		_refineFX->VariableByName("interpolationSourceTex")->AsUAV()->SetValue(interpolationSourceTex->CreateTextureView());
		_refineFX->VariableByName("sampleCoordsTex")->AsShaderResource()->SetValue(sampleCoordsTex->CreateTextureView());
		_refineFX->VariableByName("cameraDepthTex")->AsShaderResource()->SetValue(cameraDepthTex->CreateTextureView());

		float depthBreakThreshold = 0.5f / (camera->Far() - camera->Near());
		_refineFX->VariableByName("depthBreakThreshold")->AsScalar()->SetValue(&depthBreakThreshold, sizeof(depthBreakThreshold));

		rc->SetRenderTargets({}, 0);

		_refineFX->TechniqueByName("RefineSamples")->PassByIndex(0)->Bind();
		rc->Compute(_maxSamplesPerLine / std::max<int32_t>(_initalSampleStep, 32), _numSampleLines, 1);
		_refineFX->TechniqueByName("RefineSamples")->PassByIndex(0)->UnBind();

		return interpolationSourceTex;
	}

	void AtmosphereRendering::MarkRayMarchingSamples(const Ptr<Texture> & interpolationSourceTex, const Ptr<Texture> & depthStencilTex)
	{
		auto rc = Global::GetRenderEngine()->GetRenderContext();

		_fx->VariableByName("interpolationSourceTex")->AsShaderResource()->SetValue(interpolationSourceTex->CreateTextureView());

		rc->SetRenderTargets({}, 0);
		
		RenderQuad(
			_fx->TechniqueByName("MarkRayMarchingSamples"),
			0, 0,
			_maxSamplesPerLine, _numSampleLines,
			depthStencilTex->CreateTextureView());
	}

	void AtmosphereRendering::DoRayMarching(
		const Ptr<Texture> & sampleCoordsTex,
		const Ptr<Texture> & cameraDepthTex,
		const Ptr<Texture> & depthStencilTex,
		const Ptr<Camera> & camera,
		Ptr<Texture> & outLightAccumTex,
		Ptr<Texture> & outAttenuationTex)
	{
		TextureDesc texDesc;
		texDesc.width = _maxSamplesPerLine;
		texDesc.height = _numSampleLines;
		texDesc.arraySize = 1;
		texDesc.mipLevels = 1;
		texDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET;
		texDesc.cpuAccess = 0;
		texDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		texDesc.sampleCount = 1;
		texDesc.sampleQuality = 0;
		texDesc.type = TEXTURE_2D;

		auto lightAccumTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		auto attenuationTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		_fx->VariableByName("sampleCoordsTex")->AsShaderResource()->SetValue(sampleCoordsTex->CreateTextureView());
		_fx->VariableByName("cameraDepthTex")->AsShaderResource()->SetValue(cameraDepthTex->CreateTextureView());
		_fx->VariableByName("opticalDepthLUT")->AsShaderResource()->SetValue(_opticalDepthLUT->CreateTextureView());
		_fx->VariableByName("inScatteringLUTR")->AsShaderResource()->SetValue(_inScatteringLUTR->CreateTextureView(0, 1, 0, 0));
		_fx->VariableByName("inScatteringLUTM")->AsShaderResource()->SetValue(_inScatteringLUTM->CreateTextureView(0, 1, 0, 0));

		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());
		_fx->VariableByName("view")->AsScalar()->SetValue(&camera->ViewMatrix());
		auto viewXM = XMLoadFloat4x4(&camera->ViewMatrix());
		auto invViewXM = XMMatrixInverse(&XMMatrixDeterminant(viewXM), viewXM);
		XMFLOAT4X4 invView;
		XMStoreFloat4x4(&invView, invViewXM);
		_fx->VariableByName("invView")->AsScalar()->SetValue(&invView);

		_fx->VariableByName("lightRadiance")->AsScalar()->SetValue(&_sunRadiance);
		_fx->VariableByName("lightDirection")->AsScalar()->SetValue(&_sunDirection);
		_fx->VariableByName("cameraPos")->AsScalar()->SetValue(&camera->Pos());
		float2 cameraNearFar = float2(camera->Near(), camera->Far());
		_fx->VariableByName("cameraNearFar")->AsScalar()->SetValue(&cameraNearFar);

		float3 earthCenter = float3(0.0f, -_earthRadius, 0.0f);
		_fx->VariableByName("earthCenter")->AsScalar()->SetValue(&earthCenter);
		_fx->VariableByName("scatteringR")->AsScalar()->SetValue(&_scatteringR);
		_fx->VariableByName("scatteringM")->AsScalar()->SetValue(&_scatteringM);
		_fx->VariableByName("attenuationR")->AsScalar()->SetValue(&_attenuationR);
		_fx->VariableByName("attenuationM")->AsScalar()->SetValue(&_attenuationM);
		_fx->VariableByName("phaseG_M")->AsScalar()->SetValue(&_phaseG_M);

		//int4 shadowConfig = 0;
		//if (_sun->IsCastShadow())
		//{
		//	shadowConfig.x = 1;
		//	shadowConfig.y = _sun->GetShadowTechnique()->RenderTechnique()->Type();
		//	_sun->GetShadowTechnique()->BindShadowRenderParams(_fx, _sun, camera);
		//}
		//_fx->VariableByName("shadowConfig")->AsScalar()->SetValue(&shadowConfig, sizeof(shadowConfig));

		auto rc = Global::GetRenderEngine()->GetRenderContext();
		
		rc->SetRenderTargets({ lightAccumTex->CreateTextureView(), attenuationTex->CreateTextureView() }, 0);

		RenderQuad(
			_fx->TechniqueByName("RayMarching"),
			0, 0,
			texDesc.width, texDesc.height,
			//1280, 720);
			depthStencilTex->CreateTextureView());

		outLightAccumTex = lightAccumTex;
		outAttenuationTex = attenuationTex;
	}

	void AtmosphereRendering::InterpolateRestSamples(
		const Ptr<Texture> & interpolationSourceTex,
		const Ptr<Texture> & cameraDepthTex,
		const Ptr<Texture> & lightAccumTex,
		const Ptr<Texture> & attenuationTex,
		Ptr<Texture> & outLightAccumTex,
		Ptr<Texture> & outAttenuationTex)
	{
		TextureDesc texDesc = lightAccumTex->Desc();

		outLightAccumTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);
		outAttenuationTex = Global::GetRenderEngine()->GetRenderFactory()->GetTexturePooled(texDesc);

		_fx->VariableByName("interpolationSourceTex")->AsShaderResource()->SetValue(interpolationSourceTex->CreateTextureView());
		_fx->VariableByName("cameraDepthTex")->AsShaderResource()->SetValue(cameraDepthTex->CreateTextureView());
		_fx->VariableByName("lightAccumTex")->AsShaderResource()->SetValue(lightAccumTex->CreateTextureView());
		_fx->VariableByName("attenuationTex")->AsShaderResource()->SetValue(attenuationTex->CreateTextureView());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ outLightAccumTex->CreateTextureView(), outAttenuationTex->CreateTextureView() }, 0);

		RenderQuad(_fx->TechniqueByName("InterpolateRestSamples"), 0, 0, texDesc.width, texDesc.height);
	}

	void AtmosphereRendering::UnWarpSamples(
		const Ptr<Texture> & lightAccumTex,
		const Ptr<Texture> & attenuationTex,
		const Ptr<Texture> & sceneTex,
		const ResourceView & target)
	{
		_fx->VariableByName("lightAccumTex")->AsShaderResource()->SetValue(lightAccumTex->CreateTextureView());
		_fx->VariableByName("attenuationTex")->AsShaderResource()->SetValue(attenuationTex->CreateTextureView());
		_fx->VariableByName("sceneTex")->AsShaderResource()->SetValue(sceneTex->CreateTextureView());

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ target }, 0);

		RenderQuad(_fx->TechniqueByName("UnWarpSamples"), 0, 0, sceneTex->Desc().width, sceneTex->Desc().height);
	}

	void AtmosphereRendering::RenderSun(
		const float2 & lightPosH,
		const Ptr<Camera> & camera,
		const ResourceView & target,
		const Ptr<Texture> & cullDepthStencil)
	{
		float sunAngularRadius = _sunRenderRadius / 60.0f * (XM_2PI / 360.0f);
		float sunRadius = std::tan(sunAngularRadius);
		_fx->VariableByName("sunRadius")->AsScalar()->SetValue(&sunRadius);

		_fx->VariableByName("proj")->AsScalar()->SetValue(&camera->ProjMatrix());

		_fx->VariableByName("sunRadiance")->AsScalar()->SetValue(&_sunRenderColor);
		_fx->VariableByName("lightPosH")->AsScalar()->SetValue(&lightPosH);

		auto rc = Global::GetRenderEngine()->GetRenderContext();

		rc->SetRenderTargets({ target }, 0);
		rc->SetDepthStencil(cullDepthStencil->CreateTextureView(0, 1, 0, 1, RENDER_FORMAT_D24_UNORM_S8_UINT));

		auto ri = std::make_shared<RenderInput>();
		ri->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		rc->SetRenderInput(ri);

		auto targetTex = std::static_pointer_cast<Texture>(target.resource);

		auto preVP = rc->GetViewport();
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(targetTex->Desc().width);
		vp.height = static_cast<float>(targetTex->Desc().height);
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		rc->SetViewport(vp);

		_fx->TechniqueByName("RenderSun")->PassByIndex(0)->Bind();
		rc->DrawVertices(4, 0);
		_fx->TechniqueByName("RenderSun")->PassByIndex(0)->UnBind();

		rc->SetViewport(preVP);
	}
}
