#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\D3D11\D3D11RenderEngine.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11BlendState.h"
#include "ToyGE\D3D11\D3D11DepthStencilState.h"
#include "ToyGE\D3D11\D3D11RasterizerState.h"
#include "ToyGE\D3D11\D3D11Util.h"

namespace ToyGE
{
	void D3D11RenderContext::DoSetViewport(const RenderViewport & viewport)
	{
		D3D11_VIEWPORT d3dVP;
		d3dVP.TopLeftX = viewport.topLeftX;
		d3dVP.TopLeftY = viewport.topLeftY;
		d3dVP.Width = viewport.width;
		d3dVP.Height = viewport.height;
		d3dVP.MinDepth = viewport.minDepth;
		d3dVP.MaxDepth = viewport.maxDepth;

		D3D11RenderEngine::d3d11DeviceContext->RSSetViewports(1, &d3dVP);

		_state.viewport = viewport;
	}

	void D3D11RenderContext::DoSetVertexBuffer(const std::vector<Ptr<VertexBuffer>> & vertexBuffers, const std::vector<int32_t> & byteOffsets)
	{
		std::vector<ID3D11Buffer*> d3dVertexBuffers;
		std::vector<uint32_t> strides;
		std::vector<uint32_t> offsets;
		int32_t index = 0;
		for (auto & vb : vertexBuffers)
		{
			if (!vb)
			{
				d3dVertexBuffers.push_back(nullptr);
				continue;
			}

			auto d3dVB = std::dynamic_pointer_cast<D3D11RenderBuffer>(vb);
			d3dVertexBuffers.push_back(d3dVB->GetHardwareBuffer().get());
			strides.push_back(static_cast<uint32_t>(d3dVB->GetDesc().elementSize));
			offsets.push_back(static_cast<uint32_t>(byteOffsets[index++]) );
		}

		if(d3dVertexBuffers.size() > 0)
			D3D11RenderEngine::d3d11DeviceContext->IASetVertexBuffers(0, static_cast<UINT>(d3dVertexBuffers.size()), &d3dVertexBuffers[0], &strides[0], &offsets[0]);
		else
			D3D11RenderEngine::d3d11DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	}

	void D3D11RenderContext::DoSetIndexBuffer(const Ptr<RenderBuffer> & indexBuffer, int32_t bytesOffset)
	{
		if (indexBuffer)
		{
			auto d3dBuffer = std::dynamic_pointer_cast<D3D11RenderBuffer>(indexBuffer);
			DXGI_FORMAT ibFmt = DXGI_FORMAT_UNKNOWN;
			if (4 == d3dBuffer->GetDesc().elementSize)
				ibFmt = DXGI_FORMAT_R32_UINT;
			else if (2 == d3dBuffer->GetDesc().elementSize)
				ibFmt = DXGI_FORMAT_R16_UINT;

			D3D11RenderEngine::d3d11DeviceContext->IASetIndexBuffer(d3dBuffer->GetHardwareBuffer().get(), ibFmt, static_cast<uint32_t>(bytesOffset));
		}
		else
		{
			D3D11RenderEngine::d3d11DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}
	}

	void D3D11RenderContext::DoSetVertexInputLayout(const Ptr<VertexInputLayout> & vertexInputLayout)
	{
		if (vertexInputLayout)
		{
			auto d3dVertexInputLayout = std::static_pointer_cast<D3D11VertexInputLayout>(vertexInputLayout);
			D3D11RenderEngine::d3d11DeviceContext->IASetInputLayout(d3dVertexInputLayout->hardwareInputLayout.get());
		}
		else
		{
			D3D11RenderEngine::d3d11DeviceContext->IASetInputLayout(nullptr);
		}
	}

	void D3D11RenderContext::DoSetPrimitiveTopology(PrimitiveTopology primitiveTopology)
	{
		D3D11RenderEngine::d3d11DeviceContext->IASetPrimitiveTopology(GetD3DPrimitiveTopology(primitiveTopology));
	}

	void D3D11RenderContext::DoSetRenderTargetsAndDepthStencil(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil)
	{
		// RTVs
		std::vector<ID3D11RenderTargetView*> d3dRenderTargets;
		std::transform(renderTargets.begin(), renderTargets.end(), std::back_inserter(d3dRenderTargets), 
			[](const Ptr<RenderTargetView> & rtv) -> ID3D11RenderTargetView*
		{
			if (!rtv)
				return nullptr;

			if (rtv->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11TextureRenderTargetView>(rtv);
				return d3d11RTV->hardwareRTV.get();
			}
			else
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11BufferRenderTargetView>(rtv);
				return d3d11RTV->hardwareRTV.get();
			}
		});

		// DSV
		ID3D11DepthStencilView *d3dDepthStencil = nullptr;
		if (depthStencil)
		{
			auto d3d11DSV = std::static_pointer_cast<D3D11TextureDepthStencilView>(depthStencil);
			d3dDepthStencil = d3d11DSV->hardwareDSV.get();
		}

		if (d3dRenderTargets.size() > 0)
			D3D11RenderEngine::d3d11DeviceContext->OMSetRenderTargets(static_cast<UINT>(d3dRenderTargets.size()), &d3dRenderTargets[0], d3dDepthStencil);
		else
			D3D11RenderEngine::d3d11DeviceContext->OMSetRenderTargets(0, nullptr, d3dDepthStencil);
	}

	void D3D11RenderContext::DoSetShaderProgram(ShaderType shaderType, const Ptr<ShaderProgram> & program)
	{
		switch (shaderType)
		{
		case SHADER_VS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11VertexShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->VSSetShader(d3dShader->GetHardwareVertexShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->VSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_HS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11HullShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->HSSetShader(d3dShader->GetHardwareHullShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->HSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_DS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11DomainShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->DSSetShader(d3dShader->GetHardwareDomainShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->DSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_GS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11GeometryShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->GSSetShader(d3dShader->GetHardwareGeometryShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->GSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_PS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11PixelShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->PSSetShader(d3dShader->GetHardwarePixelShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->PSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_CS:
		{
			if (program)
			{
				auto d3dShader = std::static_pointer_cast<D3D11ComputeShaderProgram>(program);
				D3D11RenderEngine::d3d11DeviceContext->CSSetShader(d3dShader->GetHardwareComputeShader().get(), 0, 0);
			}
			else
				D3D11RenderEngine::d3d11DeviceContext->CSSetShader(nullptr, 0, 0);
			break;
		}
		default:
			break;
		}
	}

	void D3D11RenderContext::DoSetSRVs(ShaderType shaderType, const std::vector<Ptr<ShaderResourceView>> & srvs, int32_t offset)
	{
		typedef void (D3D11SetSRVFuncType)(ID3D11DeviceContext *, uint32_t offset, uint32_t numViews, ID3D11ShaderResourceView * const *);

		static std::map<ShaderType, std::function<D3D11SetSRVFuncType>> D3D11SetSRVFuncMap =
		{
			{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetShaderResources) },
			{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetShaderResources) },
			{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetShaderResources) },
			{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetShaderResources) },
			{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetShaderResources) },
			{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetShaderResources) }
		};


		std::vector<ID3D11ShaderResourceView*> d3d11SRVList;
		for (auto & srv : srvs)
		{
			if (srv)
			{
				if (srv->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
				{
					auto d3d11SRV = std::static_pointer_cast<D3D11TextureShaderResourceView>(srv);
					d3d11SRVList.push_back(d3d11SRV->hardwareSRV.get());
				}
				else
				{
					auto d3d11SRV = std::static_pointer_cast<D3D11BufferShaderResourceView>(srv);
					d3d11SRVList.push_back(d3d11SRV->hardwareSRV.get());
				}
			}
			else
				d3d11SRVList.push_back(nullptr);
		}

		if (d3d11SRVList.size() > 0)
			D3D11SetSRVFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, static_cast<UINT>(d3d11SRVList.size()), &d3d11SRVList[0]);
		else
			D3D11SetSRVFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::DoSetUAVs(ShaderType shaderType, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts, int32_t offset)
	{
		typedef void (D3D11SetUAVFuncType)(ID3D11DeviceContext *, uint32_t offset, uint32_t numViews, ID3D11UnorderedAccessView * const *, const uint32_t *);

		static std::map<ShaderType, std::function<D3D11SetUAVFuncType>> D3D11SetUAVFuncMap =
		{
			{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetUnorderedAccessViews) }
		};


		std::vector<ID3D11UnorderedAccessView*> d3d11UAVList;
		std::vector<uint32_t> initalCounts;
		int32_t index = 0;

		for (auto & uav : uavs)
		{
			if (uav)
			{
				if (uav->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
				{
					auto d3d11UAV = std::static_pointer_cast<D3D11TextureUnorderedAccessView>(uav);
					d3d11UAVList.push_back(d3d11UAV->hardwareUAV.get());
				}
				else
				{
					auto d3d11UAV = std::static_pointer_cast<D3D11BufferUnorderedAccessView>(uav);
					d3d11UAVList.push_back(d3d11UAV->hardwareUAV.get());
				}
			}
			else
				d3d11UAVList.push_back(nullptr);

			initalCounts.push_back(static_cast<uint32_t>(uavInitialCounts[index++]));
		}

		if (d3d11UAVList.size() > 0)
			D3D11SetUAVFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, static_cast<UINT>(d3d11UAVList.size()), &d3d11UAVList[0], &initalCounts[0]);
		else
			D3D11SetUAVFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, 0, nullptr, nullptr);
	}

	void D3D11RenderContext::DoSetSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset)
	{
		typedef void (D3D11SetSamplerFuncType)(ID3D11DeviceContext *, uint32_t offset, uint32_t numSamplers, ID3D11SamplerState * const *);

		static std::map<ShaderType, std::function<D3D11SetSamplerFuncType>> D3D11SetSamplerFuncMap =
		{
			{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetSamplers) },
			{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetSamplers) },
			{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetSamplers) },
			{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetSamplers) },
			{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetSamplers) },
			{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetSamplers) }
		};


		std::vector<ID3D11SamplerState*> d3d11SamplerList;

		for (auto & sampler : samplers)
		{
			if (sampler)
			{
				Ptr<D3D11Sampler> d3d11Sampler = std::static_pointer_cast<D3D11Sampler>(sampler);
				d3d11SamplerList.push_back(d3d11Sampler->GetHardwareSampler().get());
			}
			else
				d3d11SamplerList.push_back(nullptr);
		}

		if (d3d11SamplerList.size() > 0)
			D3D11SetSamplerFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, static_cast<UINT>(d3d11SamplerList.size()), &d3d11SamplerList[0]);
		else
			D3D11SetSamplerFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::DoSetRTVsAndUAVs(const std::vector<Ptr<RenderTargetView>> & renderTargets, const Ptr<DepthStencilView> & depthStencil, const std::vector<Ptr<UnorderedAccessView>> & uavs, const std::vector<int32_t> & uavInitialCounts)
	{
		// RTVs
		std::vector<ID3D11RenderTargetView*> d3d11RTVList;
		std::transform(renderTargets.begin(), renderTargets.end(), std::back_inserter(d3d11RTVList),
			[](const Ptr<RenderTargetView> & rtv) -> ID3D11RenderTargetView*
		{
			if (!rtv)
				return nullptr;

			if (rtv->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11TextureRenderTargetView>(rtv);
				return d3d11RTV->hardwareRTV.get();
			}
			else
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11BufferRenderTargetView>(rtv);
				return d3d11RTV->hardwareRTV.get();
			}
		});

		// DSV
		ID3D11DepthStencilView *d3dDepthStencil = nullptr;
		if (depthStencil)
		{
			auto d3d11DSV = std::static_pointer_cast<D3D11TextureDepthStencilView>(depthStencil);
			d3dDepthStencil = d3d11DSV->hardwareDSV.get();
		}

		// UAVs
		std::vector<ID3D11UnorderedAccessView*> d3d11UAVList;
		std::vector<uint32_t> initalCounts;
		int32_t index = 0;

		for (auto & uav : uavs)
		{
			if (uav)
			{
				if (uav->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
				{
					auto d3d11UAV = std::static_pointer_cast<D3D11TextureUnorderedAccessView>(uav);
					d3d11UAVList.push_back(d3d11UAV->hardwareUAV.get());
				}
				else
				{
					auto d3d11UAV = std::static_pointer_cast<D3D11BufferUnorderedAccessView>(uav);
					d3d11UAVList.push_back(d3d11UAV->hardwareUAV.get());
				}
			}
			else
				d3d11UAVList.push_back(nullptr);

			initalCounts.push_back(static_cast<uint32_t>(uavInitialCounts[index++]));
		}

		// Reset RTVs
		D3D11RenderEngine::d3d11DeviceContext.get()->OMSetRenderTargets(static_cast<uint32_t>(d3d11RTVList.size()), d3d11RTVList.size() > 0 ? &d3d11RTVList[0] : nullptr, d3dDepthStencil);

		// Erase back null
		while (d3d11RTVList.size() > 0 && d3d11RTVList.back() == nullptr)
			d3d11RTVList.pop_back();

		// Do set
		D3D11RenderEngine::d3d11DeviceContext.get()->OMSetRenderTargetsAndUnorderedAccessViews(
			static_cast<UINT>(d3d11RTVList.size()),
			d3d11RTVList.size() > 0 ? &d3d11RTVList[0] : nullptr,
			d3dDepthStencil,
			static_cast<uint32_t>(d3d11RTVList.size()),
			static_cast<UINT>(d3d11UAVList.size()),
			&d3d11UAVList[0], 
			&initalCounts[0]);
	}

	void D3D11RenderContext::DoSetCBs(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset)
	{
		typedef void (D3D11SetCBFuncType)(ID3D11DeviceContext *, uint32_t offset, uint32_t numBuffers, ID3D11Buffer * const *);

		static std::map<ShaderType, std::function<D3D11SetCBFuncType>> D3D11SetCBFuncMap =
		{
			{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetConstantBuffers) },
			{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetConstantBuffers) },
			{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetConstantBuffers) },
			{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetConstantBuffers) },
			{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetConstantBuffers) },
			{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetConstantBuffers) }
		};


		std::vector<ID3D11Buffer*> d3d11BufferList;

		for (auto & buffer : buffers)
		{
			if (buffer)
			{
				Ptr<D3D11RenderBuffer> d3d11Buffer = std::dynamic_pointer_cast<D3D11RenderBuffer>(buffer);
				d3d11BufferList.push_back(d3d11Buffer->GetHardwareBuffer().get());
			}
			else
				d3d11BufferList.push_back(nullptr);
		}

		if (d3d11BufferList.size() > 0)
			D3D11SetCBFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, static_cast<UINT>(d3d11BufferList.size()), &d3d11BufferList[0]);
		else
			D3D11SetCBFuncMap[shaderType](D3D11RenderEngine::d3d11DeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::DoSetBlendState(const Ptr<BlendState> & state, const float4 & blendFactors, uint32_t sampleMask)
	{
		if (state)
		{
			Ptr<D3D11BlendState> d3d11BlendState = std::static_pointer_cast<D3D11BlendState>(state);
			D3D11RenderEngine::d3d11DeviceContext->OMSetBlendState(d3d11BlendState->GetHardwareBlendState().get(), &blendFactors.x(), sampleMask);
		}
		else
			D3D11RenderEngine::d3d11DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	}

	void D3D11RenderContext::DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef)
	{
		if (state)
		{
			Ptr<D3D11DepthStencilState> d3d11DepthStencilState = std::static_pointer_cast<D3D11DepthStencilState>(state);
			D3D11RenderEngine::d3d11DeviceContext->OMSetDepthStencilState(d3d11DepthStencilState->GetHardwareDepthStencilState().get(), steincilRef);
		}
		else
			D3D11RenderEngine::d3d11DeviceContext->OMSetDepthStencilState(nullptr, 0);
	}

	void D3D11RenderContext::DoSetRasterizerState(const Ptr<RasterizerState> & state)
	{
		if (state)
		{
			Ptr<D3D11RasterizerState> d3d11RasterizerState = std::static_pointer_cast<D3D11RasterizerState>(state);
			D3D11RenderEngine::d3d11DeviceContext->RSSetState(d3d11RasterizerState->GetHardwareRasterizerState().get());
		}
		else
			D3D11RenderEngine::d3d11DeviceContext->RSSetState(nullptr);
	}

	void D3D11RenderContext::DoDrawVertices(int32_t numVertices, int32_t vertexStart)
	{
		D3D11RenderEngine::d3d11DeviceContext->Draw(static_cast<uint32_t>(numVertices), static_cast<uint32_t>(vertexStart));
	}

	void D3D11RenderContext::DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase)
	{
		D3D11RenderEngine::d3d11DeviceContext->DrawIndexed(numIndices, indexStart, indexBase);
	}

	void D3D11RenderContext::DoDrawInstanced(int32_t numVerticesPerInstance, int32_t numInstances, int32_t vertexStart, int32_t instanceBase)
	{
		D3D11RenderEngine::d3d11DeviceContext->DrawInstanced((UINT)numVerticesPerInstance, (UINT)numInstances, (UINT)vertexStart, (UINT)instanceBase);
	}

	void D3D11RenderContext::DoDrawIndexedInstanced(int32_t numIndicesPerInstance, int32_t numInstances, int32_t indexStart, int32_t indexBase, int32_t instanceBase)
	{
		D3D11RenderEngine::d3d11DeviceContext->DrawIndexedInstanced((UINT)numIndicesPerInstance, (UINT)numInstances, (UINT)indexStart, (INT)indexBase, (UINT)instanceBase);
	}

	void D3D11RenderContext::DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset)
	{
		auto d3d11Buffer = std::dynamic_pointer_cast<D3D11RenderBuffer>(indirectArgsBuffer);

		D3D11RenderEngine::d3d11DeviceContext->DrawInstancedIndirect(d3d11Buffer->GetHardwareBuffer().get(), bytesOffset);
	}

	void D3D11RenderContext::DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ)
	{
		D3D11RenderEngine::d3d11DeviceContext->Dispatch(groupX, groupY, groupZ);
	}

	void D3D11RenderContext::DoClearRenderTarget(const Ptr<RenderTargetView> & renderTarget, const float4 & color)
	{
		if (renderTarget)
		{
			if (renderTarget->GetResource()->GetResourceType() == RenderResourceType::RRT_TEXTURE)
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11TextureRenderTargetView>(renderTarget);
				//return d3d11RTV->hardwareRTV.get();
				D3D11RenderEngine::d3d11DeviceContext->ClearRenderTargetView(d3d11RTV->hardwareRTV.get(), &color[0]);
			}
			else
			{
				auto d3d11RTV = std::static_pointer_cast<D3D11BufferRenderTargetView>(renderTarget);
				D3D11RenderEngine::d3d11DeviceContext->ClearRenderTargetView(d3d11RTV->hardwareRTV.get(), &color[0]);
			}
			//auto d3d11RTV = std::static_pointer_cast<D3D11RenderTargetView>(renderTarget);
		}
	}

	void D3D11RenderContext::DoClearDepthStencil(const Ptr<DepthStencilView> & depthStencil, float depth, uint8_t stencil)
	{
		if(depthStencil)
		{ 
			auto d3d11DSV = std::static_pointer_cast<D3D11TextureDepthStencilView>(depthStencil);
			D3D11RenderEngine::d3d11DeviceContext->ClearDepthStencilView(d3d11DSV->hardwareDSV.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
		}
	}
}