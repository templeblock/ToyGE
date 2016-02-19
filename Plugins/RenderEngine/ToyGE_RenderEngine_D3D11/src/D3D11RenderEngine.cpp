#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\D3D11\D3D11RenderFactory.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"

#include "AntTweakBar.h"

namespace ToyGE
{
	void CreateRenderEngine(ToyGE::RenderEngine **ppRenderEngine)
	{
		using ToyGE::D3D11RenderEngine;
		*ppRenderEngine = new D3D11RenderEngine();
	}

	Ptr<DllObj> D3D11RenderEngine::dxgi_Dll;
	Ptr<DllObj> D3D11RenderEngine::d3d11_Dll;
	Ptr<DllObj> D3D11RenderEngine::d3dCompiler_Dll;

	Ptr<IDXGIFactory1>				D3D11RenderEngine::d3d11DXGIFactory;
	std::vector<Ptr<IDXGIAdapter1>>	D3D11RenderEngine::d3d11DXGIAdapterList;
	std::vector<Ptr<IDXGIOutput>>	D3D11RenderEngine::d3d11DXGIOutputsList;
	int32_t							D3D11RenderEngine::d3d11DXGISelectedAdapterIndex;
	Ptr<IDXGISwapChain>				D3D11RenderEngine::d3d11DXGISwapChain;
	Ptr<ID3D11Device>				D3D11RenderEngine::d3d11Device;
	Ptr<ID3D11DeviceContext>		D3D11RenderEngine::d3d11DeviceContext;

	D3D11RenderEngine::D3D11RenderEngine()
	{
		_renderContext = std::make_shared<D3D11RenderContext>();
		_renderFactory = std::make_shared<D3D11RenderFactory>();
	}

	D3D11RenderEngine::~D3D11RenderEngine()
	{
		d3d11DXGIFactory = nullptr;
		d3d11DXGIAdapterList.clear();
		d3d11DXGISwapChain = nullptr;
		d3d11Device = nullptr;
		d3d11DeviceContext = nullptr;

		dxgi_Dll = nullptr;
		d3d11_Dll = nullptr;
		d3dCompiler_Dll = nullptr;
	}

	void D3D11RenderEngine::Init(const RenderEngineInitParams & initParams)
	{
		RenderEngine::Init(initParams);

		// Load dlls
		{
			auto & platformFactory = Global::GetPlatform();

			dxgi_Dll = platformFactory->FindDll("dxgi.dll");
			if (!dxgi_Dll)
			{
				ToyGE_LOG(LT_ERROR, "Cannot find module=%s!", "dxgi.dll");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}

			d3d11_Dll = platformFactory->FindDll("d3d11.dll");
			if (!d3d11_Dll)
			{
				ToyGE_LOG(LT_ERROR, "Cannot find module=%s!", "d3d11.dll");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}

			d3dCompiler_Dll = platformFactory->FindDll("d3dCompiler_47.dll");
			if (!d3dCompiler_Dll)
			{
				ToyGE_LOG(LT_ERROR, "Cannot find module=%s!", "d3dCompiler_47.dll");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}
		}

		// Init d3d11 factory
		{
			auto createDxgiFactoryFunc = reinterpret_cast<decltype(CreateDXGIFactory1) *> (dxgi_Dll->GetProcAddress("CreateDXGIFactory1"));
			IDXGIFactory1 * pDxgiFactory;
			createDxgiFactoryFunc(IID_IDXGIFactory1, reinterpret_cast<void**>(&pDxgiFactory));
			d3d11DXGIFactory = MakeComShared(pDxgiFactory);
		}

		// Init adapter
		{
			// Enum all adpaters
			d3d11DXGIAdapterList.clear();
			IDXGIAdapter1 * pAdapter = nullptr;
			int32_t adapterIndex = 0;
			while (d3d11DXGIFactory->EnumAdapters1(adapterIndex++, &pAdapter) == S_OK)
			{
				d3d11DXGIAdapterList.push_back(MakeComShared(pAdapter));
			}

			if (d3d11DXGIAdapterList.size() <= 0)
			{
				ToyGE_LOG(LT_ERROR, "Cannot find available adapter!");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}

			// Select adapter
			
			if (initParams.adapterIndex >= 0 && initParams.adapterIndex < (int32_t)d3d11DXGIAdapterList.size())
			{
				d3d11DXGISelectedAdapterIndex = initParams.adapterIndex;
			}
			else if (initParams.adapterSelectKey.size() > 0)
			{
				auto selectKey = initParams.adapterSelectKey;
				std::transform(selectKey.begin(), selectKey.end(), selectKey.begin(), ::tolower);
				//auto u16SelectKey = StringConvert<StringEncode::UTF_8, StringEncode::UTF_16>( initParams.adapterSelectKey );

				int32_t adapterIndex = 0;
				for (auto & adapter : d3d11DXGIAdapterList)
				{
					DXGI_ADAPTER_DESC adapterDesc;
					adapter->GetDesc(&adapterDesc);

					auto description = StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(reinterpret_cast<const char16_t*>(adapterDesc.Description));
					std::transform(description.begin(), description.end(), description.begin(), ::tolower);

					if (description.find(selectKey) != std::string::npos)
					{
						d3d11DXGISelectedAdapterIndex = adapterIndex;
						break;
					}
					++adapterIndex;
				}
			}
			else
			{
				d3d11DXGISelectedAdapterIndex = 0;
			}

			Ptr<IDXGIAdapter1> selectAdapter = d3d11DXGIAdapterList[d3d11DXGISelectedAdapterIndex];
			DXGI_ADAPTER_DESC selectedAdapterDesc;
			selectAdapter->GetDesc(&selectedAdapterDesc);
			_adapter.description = StringConvert<StringEncode::UTF_16, StringEncode::UTF_8>(reinterpret_cast<const char16_t*>(selectedAdapterDesc.Description));

			ToyGE_LOG(LT_INFO, "Adapter selected! adapter=%s", _adapter.description.c_str());
		}

		// Init outputs
		{
			Ptr<IDXGIAdapter1> selectAdapter = d3d11DXGIAdapterList[d3d11DXGISelectedAdapterIndex];

			IDXGIOutput * pOutput = nullptr;
			int32_t outputIndex = 0;
			while (selectAdapter->EnumOutputs(outputIndex++, &pOutput) == S_OK)
			{
				d3d11DXGIOutputsList.push_back(MakeComShared(pOutput));
			}

			_outputModesList.resize(d3d11DXGIOutputsList.size());
			outputIndex = 0;
			for (auto & output : d3d11DXGIOutputsList)
			{
				UINT numModes = 0;
				output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);
				std::vector<DXGI_MODE_DESC> dxgiModes(numModes);
				output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, &dxgiModes[0]);

				for (auto & dxgiMode : dxgiModes)
				{
					RenderDeviceOutputMode mode;
					mode.width = (int32_t)dxgiMode.Width;
					mode.height = (int32_t)dxgiMode.Height;
					mode.refreshRateNumerator = (int32_t)dxgiMode.RefreshRate.Numerator;
					mode.refreshRateDenominator = (int32_t)dxgiMode.RefreshRate.Denominator;
					mode.scalineOrder = (OutputScalineOrder)dxgiMode.ScanlineOrdering;
					mode.scaling = (OutputScaling)dxgiMode.Scaling;
					_outputModesList[outputIndex].push_back(mode);
				}

				++outputIndex;
			}
		}

		// Init d3d11 device
		{
			auto createDeviceFunc = reinterpret_cast<decltype(::D3D11CreateDevice)*>(d3d11_Dll->GetProcAddress("D3D11CreateDevice"));

			UINT createDeviceFlags = 0;
			if(initParams.bGrapicsEngineDebug)
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

			ID3D11Device *pD3d11Device = nullptr;
			ID3D11DeviceContext *pD3d11DeviceContext = nullptr;
			D3D_FEATURE_LEVEL featureLevel;

			if (createDeviceFunc(d3d11DXGIAdapterList[d3d11DXGISelectedAdapterIndex].get(), D3D_DRIVER_TYPE_UNKNOWN, 0,
				createDeviceFlags,
				0, 0, D3D11_SDK_VERSION,
				&pD3d11Device, &featureLevel, &pD3d11DeviceContext) != S_OK)
			{
				ToyGE_LOG(LT_ERROR, "Cannot create d3d11 device!");
				ToyGE_ASSERT_FAIL("Engine init fatal error!");
			}
			d3d11Device = MakeComShared(pD3d11Device);
			d3d11DeviceContext = MakeComShared(pD3d11DeviceContext);
		}

		// Init swap chain
		{
			DXGI_SWAP_CHAIN_DESC sd;
			sd.BufferDesc.Width = Global::GetWindow()->Width();
			sd.BufferDesc.Height = Global::GetWindow()->Height();
			sd.BufferDesc.RefreshRate.Numerator = (UINT)_outputModesList[0].back().refreshRateNumerator;
			sd.BufferDesc.RefreshRate.Denominator = (UINT)_outputModesList[0].back().refreshRateDenominator;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;

			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = 2;
			sd.OutputWindow = Global::GetWindow()->WindowHandle();
			sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			sd.Windowed = true;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			IDXGISwapChain *pSwapChain = nullptr;
			d3d11DXGIFactory->CreateSwapChain(d3d11Device.get(), &sd, &pSwapChain);

			d3d11DXGISwapChain = MakeComShared(pSwapChain);
		}

		// Init back buffer
		{
			_backBuffer = GetRenderFactory()->CreateTexture(TEXTURE_2D);
			InitBackBuffer();
		}

		Global::GetWindow()->OnResizeEvent().connect(
			[&](const Ptr<Window> & window, int32_t preWidth, int32_t preHeight)
		{
			_backBuffer->Release();
			_frameBuffer->Release();
			d3d11DXGISwapChain->ResizeBuffers(2, window->Width(), window->Height(), DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
			InitBackBuffer();

			TwWindowSize(window->Width(), window->Height());
		});

		// Init AntTweakBar
		TwInit(TW_DIRECT3D11, d3d11Device.get());
		TwWindowSize(Global::GetWindow()->Width(), Global::GetWindow()->Height());

		// Process feature level
		ProcessFeatureLevel();
	}

	void D3D11RenderEngine::ProcessFeatureLevel()
	{
		auto featureLevel = d3d11Device->GetFeatureLevel();

		if (featureLevel < D3D_FEATURE_LEVEL_10_0)
		{
			ToyGE_LOG(LT_ERROR, "Unsupported graphics card %s!", _adapter.description.c_str());
			ToyGE_ASSERT_FAIL("Engine init fatal error!");
		}

		switch (featureLevel)
		{
		/*case D3D_FEATURE_LEVEL_12_1:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 12_1");
			break;
		case D3D_FEATURE_LEVEL_12_0:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 12_0");*/
			break;
		case D3D_FEATURE_LEVEL_11_1:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 11_1");
			break;
		case D3D_FEATURE_LEVEL_11_0:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 11_0");
			break;
		case D3D_FEATURE_LEVEL_10_1:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 10_1");
			break;
		case D3D_FEATURE_LEVEL_10_0:
			ToyGE_LOG(LT_INFO, "D3D Feature Level 10_0");
			break;
		default:
			break;
		}

		// Shader model
		if (featureLevel >= D3D_FEATURE_LEVEL_11_0)
			_shaderModel = SM_5;
		else if (featureLevel >= D3D_FEATURE_LEVEL_10_0)
			_shaderModel = SM_4;
	}

	void D3D11RenderEngine::SwapChain() const
	{
		d3d11DXGISwapChain->Present(0, 0);
	}

	void D3D11RenderEngine::InitBackBuffer()
	{
		ID3D11Texture2D * pBackBuffer;
		d3d11DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		auto d3d11BackBuffer = std::static_pointer_cast<D3D11Texture2D>(_backBuffer);
		d3d11BackBuffer->InitFromHardware(MakeComShared(pBackBuffer));

		auto frameBufferDesc = _backBuffer->GetDesc();
		frameBufferDesc.bindFlag = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_UNORDERED_ACCESS;
		frameBufferDesc.format = RENDER_FORMAT_R11G11B10_FLOAT;
		if(!_frameBuffer)
			_frameBuffer = GetRenderFactory()->CreateTexture(TEXTURE_2D);
		_frameBuffer->SetDesc(frameBufferDesc);
		_frameBuffer->Init();
	}

	void D3D11RenderEngine::SetFullScreen(bool bFullScreen)
	{
		if (IsFullScreen() == bFullScreen)
			return;

		RenderEngine::SetFullScreen(bFullScreen);

		if (bFullScreen)
		{
			DXGI_MODE_DESC dxgiMode;
			dxgiMode.Width = (UINT)(_outputModesList[0].back().width);
			dxgiMode.Height = (UINT)(_outputModesList[0].back().height);
			dxgiMode.RefreshRate.Numerator = (UINT)_outputModesList[0].back().refreshRateNumerator;
			dxgiMode.RefreshRate.Denominator = (UINT)_outputModesList[0].back().refreshRateDenominator;
			dxgiMode.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)_outputModesList[0].back().scalineOrder;
			dxgiMode.Scaling = (DXGI_MODE_SCALING)_outputModesList[0].back().scaling;
			d3d11DXGISwapChain->ResizeTarget(&dxgiMode);

			d3d11DXGISwapChain->SetFullscreenState((BOOL)bFullScreen, nullptr);
		}
		else
		{
			d3d11DXGISwapChain->SetFullscreenState((BOOL)bFullScreen, nullptr);

			DXGI_MODE_DESC dxgiMode;
			dxgiMode.Width = (UINT)(Global::GetConfig()->windowWidth);
			dxgiMode.Height = (UINT)(Global::GetConfig()->windowHeight);
			dxgiMode.RefreshRate.Numerator = (UINT)_outputModesList[0].back().refreshRateNumerator;
			dxgiMode.RefreshRate.Denominator = (UINT)_outputModesList[0].back().refreshRateDenominator;
			dxgiMode.ScanlineOrdering = (DXGI_MODE_SCANLINE_ORDER)_outputModesList[0].back().scalineOrder;
			dxgiMode.Scaling = (DXGI_MODE_SCALING)_outputModesList[0].back().scaling;
			d3d11DXGISwapChain->ResizeTarget(&dxgiMode);
		}
	}

}
