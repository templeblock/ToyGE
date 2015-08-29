#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\Kernel\Util.h"
#include "ToyGE\Platform\DllObj.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Platform\PlatformFactory.h"
#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\D3D11\D3D11RenderFactory.h"
#include "ToyGE\Kernel\Logger.h"

#include "AntTweakBar.h"

namespace ToyGE
{
	class D3D11DebugReporter : public DebugReporter
	{
	public:
		std::shared_ptr<ID3D11Debug> _dbg;

		D3D11DebugReporter(const std::shared_ptr<ID3D11Debug> & dbg)
			: _dbg(dbg)
		{

		}

		void Report()
		{
			_dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}
	};

	D3D11RenderEngine::D3D11RenderEngine(const Ptr<Window> & window)
		: RenderEngine(window)
	{

	}

	void D3D11RenderEngine::DoStartup()
	{
		bool dllLoadResult = LoadDlls();
		ToyGE_ASSERT(dllLoadResult);

		InitD3D11();
		InitSwapChain();
		InitBackBuffer();

		_d3dRenderContext = std::make_shared<D3D11RenderContext>(_window, _rawD3DDeviceContext);
		_renderContext = _d3dRenderContext;

		_d3dRenderContext->SetRenderTargets({ _defaultRenderTarget->CreateTextureView() }, 0);

		_d3dRenderContext->SetDepthStencil(_defaultDepthStencil->CreateTextureView());
		RenderViewport vp;
		vp.topLeftX = 0.0f;
		vp.topLeftY = 0.0f;
		vp.width = static_cast<float>(_window->Width());
		vp.height = static_cast<float>(_window->Height());
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		_d3dRenderContext->SetViewport(vp);

		_d3dRenderFactory = std::make_shared<D3D11RenderFactory>();
		_renderFactory = _d3dRenderFactory;

		_window->OnResizeEvent().connect(
			[&](const Ptr<Window> & window, int32_t preWidth, int32_t preHeight)
		{
			_defaultRenderTarget.reset();
			_defaultDepthStencil.reset();
			_d3dRenderContext->ResetRenderTargetAndDepthStencil();
			_rawSwapChain->ResizeBuffers(2, window->Width(), window->Height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);
			InitBackBuffer();

			_d3dRenderContext->SetRenderTargets({ _defaultRenderTarget->CreateTextureView() }, 0);

			_d3dRenderContext->SetDepthStencil(_defaultDepthStencil->CreateTextureView());

			TwWindowSize(window->Width(), window->Height());
		});

		//AntTweakBar
		TwInit(TW_DIRECT3D11, _rawD3DDevice.get());
		TwWindowSize(this->GetWindow()->Width(), this->GetWindow()->Height());
	}

	void D3D11RenderEngine::SwapChain() const
	{
		_rawSwapChain->Present(0, 0);
	}

	void D3D11RenderEngine::InitD3D11()
	{
		//Create DXGIFacoty
		auto createDxgiFactoryFunc = reinterpret_cast<decltype(CreateDXGIFactory1) *> (_dxgi_Dll->GetProcAddress("CreateDXGIFactory1"));
		IDXGIFactory1 *pDxgiFactory;
		createDxgiFactoryFunc(IID_IDXGIFactory1, reinterpret_cast<void**>(&pDxgiFactory));
		_rawDXGIFactory = MakeComShared(pDxgiFactory);

		//Init adapters
		IDXGIAdapter1 *pAdapter = nullptr;
		uint32_t adapterIndex = 0;
		while (_rawDXGIFactory->EnumAdapters1(adapterIndex++, &pAdapter) == S_OK)
		{
			_rawDXGIAdapterList.push_back(MakeComShared(pAdapter));
		}

		if (_rawDXGIAdapterList.size() <= 0)
		{
			Global::GetPlatformFactory()->ShowMessage("No Adapter Found !");
			abort();
		}

		//select adapter
		Ptr<IDXGIAdapter1> pSelectAdapter = nullptr;
		for (auto & adapter : _rawDXGIAdapterList)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			adapter->GetDesc(&adapterDesc);
			auto findStr = L"NVIDIA";
			//auto findStr = L"Intel";
			bool cmpResult = wcsncmp(adapterDesc.Description, findStr, wcslen(findStr)) == 0;
			if (cmpResult)
			{
				pSelectAdapter = adapter;
				_adapter.description = adapterDesc.Description;
				break;
			}
		}
		if (pSelectAdapter == nullptr)
		{
			pSelectAdapter = _rawDXGIAdapterList[0];
			DXGI_ADAPTER_DESC adapterDesc;
			pSelectAdapter->GetDesc(&adapterDesc);
			_adapter.description = adapterDesc.Description;
		}

		//Create id3d11device
		auto createDeviceFunc = reinterpret_cast<decltype(::D3D11CreateDevice)*>(_d3d11_Dll->GetProcAddress("D3D11CreateDevice"));

		ToyGE_ASSERT(createDeviceFunc);

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		ID3D11Device *pD3d11Device = nullptr;
		ID3D11DeviceContext *pD3d11DeviceContext = nullptr;
		D3D_FEATURE_LEVEL featureLevel;

		if (createDeviceFunc(pSelectAdapter.get(), D3D_DRIVER_TYPE_UNKNOWN, 0,
			createDeviceFlags,
			0, 0, D3D11_SDK_VERSION,
			&pD3d11Device, &featureLevel, &pD3d11DeviceContext) != S_OK)
		{
			Logger::LogLine("error> Could Not Create Device !");
			abort();
		}

		//ID3D11Device1 * pD3d11Device1;
		//pD3d11Device->QueryInterface(__uuidof (ID3D11Device1), (void **)&pD3d11Device1);
		//IDXGIDevice1 * pDxdiDevice;
		//pD3d11Device1->QueryInterface(__uuidof (IDXGIDevice1), (void **)&pDxdiDevice);
		//pDxdiDevice->SetMaximumFrameLatency(3);

		//pDxdiDevice->Release();
		//pD3d11Device1->Release();

		_rawD3DDevice = MakeComShared(pD3d11Device);
		_rawD3DDeviceContext = MakeComShared(pD3d11DeviceContext);

//#define D3D11_SET_DBGREPORTER
#ifdef D3D11_SET_DBGREPORTER
		ID3D11Debug *d3dDebug;
		_rawD3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
		Global::SetDebugReporter(std::make_shared<D3D11DebugReporter>(MakeComShared(d3dDebug)));
#endif
	}

	void D3D11RenderEngine::InitSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = GetWindow()->Width();
		sd.BufferDesc.Height = GetWindow()->Height();
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 2;
		sd.OutputWindow = GetWindow()->WindowHandle();
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Windowed = true;
		sd.Flags = 0;

		IDXGISwapChain *pSwapChain = nullptr;
		_rawDXGIFactory->CreateSwapChain(_rawD3DDevice.get(), &sd, &pSwapChain);

		_rawSwapChain = MakeComShared(pSwapChain);
	}

	void D3D11RenderEngine::InitBackBuffer()
	{
		ID3D11Texture2D *backBuffer;
		_rawSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		_defaultRenderTarget = D3D11Texture2D::CreateFromRawD3D(_rawD3DDevice, MakeComShared(backBuffer));

		TextureDesc textureDesc;
		textureDesc.type = TEXTURE_2D;
		textureDesc.width = GetWindow()->Width();
		textureDesc.height = GetWindow()->Height();
		textureDesc.format = RENDER_FORMAT_D24_UNORM_S8_UINT;
		textureDesc.bindFlag = TEXTURE_BIND_DEPTH_STENCIL;
		textureDesc.mipLevels = 1;
		_defaultDepthStencil = std::make_shared<D3D11Texture2D>(textureDesc);
	}

	bool D3D11RenderEngine::LoadDlls()
	{
		auto &platformFactory = Global::GetPlatformFactory();

		_dxgi_Dll = platformFactory->AcquireDll("dxgi.dll");
		if (!_dxgi_Dll)
		{
			//platformFactory->ShowMessage("Can not load dxgi.dll !");
			Logger::LogLine("error> Can not load dxgi.dll !");
			return false;
		}

		_d3d11_Dll = platformFactory->AcquireDll("d3d11.dll");
		if (!_d3d11_Dll)
		{
			//platformFactory->ShowMessage("Can not load d3d11.dll !");
			Logger::LogLine("error> Can not load d3d11.dll !");
			return false;
		}

		_d3dCompiler_Dll = platformFactory->AcquireDll("d3dCompiler_47.dll");
		if (!_d3dCompiler_Dll)
		{
			//platformFactory->ShowMessage("Can not load d3dCompiler_46.dll !");
			Logger::LogLine("error> Can not load d3dCompiler_47.dll !");
			return false;
		}

		return true;
	}
}
