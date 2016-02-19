#pragma once
#ifndef D3D11RENDERENGINE_H
#define D3D11RENDERENGINE_H

#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\D3D11\D3D11PreInclude.h"

namespace ToyGE
{
	class DllObj;

	extern "C"
	{
		TOYGE_D3D11RE_API void CreateRenderEngine(ToyGE::RenderEngine **ppRenderEngine);
	}

	class D3D11RenderEngine : public RenderEngine
	{
	public:
		static Ptr<DllObj> dxgi_Dll;
		static Ptr<DllObj> d3d11_Dll;
		static Ptr<DllObj> d3dCompiler_Dll;

		static Ptr<IDXGIFactory1>				d3d11DXGIFactory;
		static std::vector<Ptr<IDXGIAdapter1>>	d3d11DXGIAdapterList;
		static std::vector<Ptr<IDXGIOutput>>	d3d11DXGIOutputsList;
		static int32_t							d3d11DXGISelectedAdapterIndex;
		static Ptr<IDXGISwapChain>				d3d11DXGISwapChain;
		static Ptr<ID3D11Device>				d3d11Device;
		static Ptr<ID3D11DeviceContext>			d3d11DeviceContext;

		D3D11RenderEngine();

		virtual ~D3D11RenderEngine();

		virtual void Init(const RenderEngineInitParams & initParams) override;

		virtual void SwapChain() const override;

		virtual void SetFullScreen(bool bFullScreen) override;

	private:
		void InitBackBuffer();

		void ProcessFeatureLevel();
	};
}

#endif