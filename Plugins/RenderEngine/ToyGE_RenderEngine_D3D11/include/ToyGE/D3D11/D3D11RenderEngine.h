#pragma once
#ifndef D3D11RENDERENGINE_H
#define D3D11RENDERENGINE_H

#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\Platform\Window.h"
#include "ToyGE\D3D11\D3D11REPreDeclare.h"

TOYGE_CORE_API extern ID3D11Debug * g_d3dDebug;

namespace ToyGE
{
	//class D3D11WindowListener : public WindowListener
	//{
	//public:
	//	D3D11WindowListener(D3D11RenderEngine & re);

	//	void OnPaint(const Ptr<Window> & window) override;
	//	void OnResize(const Ptr<Window> & window, int32_t prevWidth, int32_t prevHeight) override;
	//	void OnDestroy(const Ptr<Window> & window) override;

	//private:
	//	D3D11RenderEngine & _re;
	//};

	//using D3D11WindowListenerPtr = std::shared_ptr < D3D11WindowListener > ;

	class DllObj;
	class D3D11RenderFactory;
	class D3D11RenderContext;

	class D3D11RenderEngine : public RenderEngine
	{
		friend class D3D11WindowListener;

	public:
		D3D11RenderEngine(const Ptr<Window> & window);

		const Ptr<ID3D11Device> & RawD3DDevice()
		{
			return _rawD3DDevice;
		}

		const Ptr<ID3D11DeviceContext> & RawD3DDeviceContex()
		{
			return _rawD3DDeviceContext;
		}

	protected:
		void DoStartup();

	private:
		Ptr<DllObj> _dxgi_Dll;
		Ptr<DllObj> _d3d11_Dll;
		Ptr<DllObj> _d3dCompiler_Dll;

		Ptr<IDXGIFactory1> _rawDXGIFactory;
		std::vector<Ptr<IDXGIAdapter1>> _rawDXGIAdapterList;
		Ptr<ID3D11Device> _rawD3DDevice;
		Ptr<ID3D11DeviceContext> _rawD3DDeviceContext;

		Ptr<IDXGISwapChain> _rawSwapChain;

		//D3D11WindowListenerPtr _windowListener;
		Ptr<D3D11RenderContext> _d3dRenderContext;
		Ptr<D3D11RenderFactory> _d3dRenderFactory;

		void SwapChain() const override;

		void InitD3D11();
		void InitSwapChain();
		void InitBackBuffer();
		bool LoadDlls();
	};
}

#endif