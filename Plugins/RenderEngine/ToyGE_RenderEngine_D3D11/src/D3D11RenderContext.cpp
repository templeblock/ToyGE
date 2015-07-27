#include "ToyGE\D3D11\D3D11RenderContext.h"
#include "ToyGE\RenderEngine\RenderViewport.h"
#include "ToyGE\D3D11\D3D11Texture2D.h"
#include "ToyGE\D3D11\D3D11ShaderProgram.h"
#include "ToyGE\D3D11\D3D11RenderBuffer.h"
#include "ToyGE\D3D11\D3D11Sampler.h"
#include "ToyGE\D3D11\D3D11BlendState.h"
#include "ToyGE\D3D11\D3D11DepthStencilState.h"
#include "ToyGE\D3D11\D3D11RasterizerState.h"
#include "ToyGE\D3D11\D3D11Util.h"
#include "ToyGE\D3D11\D3D11RenderInput.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\RenderEngine\Shader.h"

namespace ToyGE
{
	std::map<ShaderType, std::function<D3D11RenderContext::SetSRVFunc>> D3D11RenderContext::_setSRVFuncMap =
	{
		{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetShaderResources) },
		{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetShaderResources) },
		{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetShaderResources) },
		{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetShaderResources) },
		{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetShaderResources) },
		{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetShaderResources) }
	};

	std::map<ShaderType, std::function<D3D11RenderContext::SetUAVFunc>> D3D11RenderContext::_setUAVFuncMap =
	{
		{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetUnorderedAccessViews) }
	};

	std::map<ShaderType, std::function<D3D11RenderContext::SetBufferFunc>> D3D11RenderContext::_setBufferFuncMap =
	{
		{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetConstantBuffers) },
		{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetConstantBuffers) },
		{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetConstantBuffers) },
		{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetConstantBuffers) },
		{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetConstantBuffers) },
		{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetConstantBuffers) }
	};

	std::map<ShaderType, std::function<D3D11RenderContext::SetSamplerFunc>> D3D11RenderContext::_setSamplerFuncMap =
	{
		{ SHADER_VS, std::mem_fn(&ID3D11DeviceContext::VSSetSamplers) },
		{ SHADER_HS, std::mem_fn(&ID3D11DeviceContext::HSSetSamplers) },
		{ SHADER_DS, std::mem_fn(&ID3D11DeviceContext::DSSetSamplers) },
		{ SHADER_GS, std::mem_fn(&ID3D11DeviceContext::GSSetSamplers) },
		{ SHADER_PS, std::mem_fn(&ID3D11DeviceContext::PSSetSamplers) },
		{ SHADER_CS, std::mem_fn(&ID3D11DeviceContext::CSSetSamplers) }
	};

	D3D11RenderContext::D3D11RenderContext(const Ptr<Window> & window, const Ptr<ID3D11DeviceContext> & deviceContext)
		: _window(window),
		_rawD3DDeviceContext(deviceContext),
		_cachedNumVBs(0)
	{

	}

	void D3D11RenderContext::DoSetViewport(const RenderViewport & viewport)
	{
		D3D11_VIEWPORT d3dVP;
		d3dVP.TopLeftX = viewport.topLeftX;
		d3dVP.TopLeftY = viewport.topLeftY;
		d3dVP.Width = viewport.width;
		d3dVP.Height = viewport.height;
		d3dVP.MinDepth = viewport.minDepth;
		d3dVP.MaxDepth = viewport.maxDepth;

		_rawD3DDeviceContext->RSSetViewports(1, &d3dVP);

		_state.viewport = viewport;
	}


	void D3D11RenderContext::DoSetRenderTargetsAndDepthStencil(const std::vector<ResourceView> & targets, const ResourceView & depthStencil)
	{
		std::vector<ID3D11RenderTargetView*> d3dRenderTargets;
		GetRawD3DRenderTargets(targets, d3dRenderTargets);

		ID3D11DepthStencilView *d3dDepthStencil = GetRawD3DDepthStencil(depthStencil);

		if (d3dRenderTargets.size() > 0)
			_rawD3DDeviceContext->OMSetRenderTargets(static_cast<UINT>(d3dRenderTargets.size()), &d3dRenderTargets[0], d3dDepthStencil);
		else
			_rawD3DDeviceContext->OMSetRenderTargets(0, nullptr, d3dDepthStencil);
	}

	void D3D11RenderContext::ResetRenderTargetAndDepthStencil()
	{
		_state.renderTargets.clear();
		_state.depthStencil = ResourceView();
		_rtsCache.clear();
		_depthStencilCache = ResourceView();
		_rawD3DDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	void D3D11RenderContext::DoClearRenderTargets(const std::vector<ResourceView> & renderTargets, const float4 & color)
	{
		std::vector<ID3D11RenderTargetView*> d3dRenderTargets;
		GetRawD3DRenderTargets(renderTargets, d3dRenderTargets);
		for (auto & rt : d3dRenderTargets)
		{
			if (rt)
				_rawD3DDeviceContext->ClearRenderTargetView(rt, &color[0]);

		}
	}

	void D3D11RenderContext::DoClearDepthStencil(const ResourceView & depthStencil, float depth, uint8_t stencil)
	{
		ID3D11DepthStencilView *d3dDepthStencil = GetRawD3DDepthStencil(depthStencil);
		if (d3dDepthStencil)
			_rawD3DDeviceContext->ClearDepthStencilView(d3dDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}

	void D3D11RenderContext::DoDrawVertices(int32_t numVertices, int32_t vertexStart)
	{
		//_rawD3DDeviceContext->Draw(_state.renderInput->NumVertices(), _state.renderInput->VertexStart());
		_rawD3DDeviceContext->Draw(static_cast<uint32_t>(numVertices), static_cast<uint32_t>(vertexStart));
	}

	void D3D11RenderContext::DoDrawIndexed(int32_t numIndices, int32_t indexStart, int32_t indexBase)
	{
		//_rawD3DDeviceContext->DrawIndexed(_state.renderInput->NumIndices(), _state.renderInput->IndexStart(), _state.renderInput->IndexBase());
		_rawD3DDeviceContext->DrawIndexed(numIndices, indexStart, indexBase);
	}

	void D3D11RenderContext::DoDrawInstancedIndirect(const Ptr<RenderBuffer> & indirectArgsBuffer, uint32_t bytesOffset)
	{
		auto d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(indirectArgsBuffer);

		_rawD3DDeviceContext->DrawInstancedIndirect(d3dBuffer->RawD3DBuffer().get(), bytesOffset);
	}

	void D3D11RenderContext::DoCompute(int32_t groupX, int32_t groupY, int32_t groupZ)
	{
		_rawD3DDeviceContext->Dispatch(groupX, groupY, groupZ);
	}

	void D3D11RenderContext::DoSetRenderInput(const Ptr<RenderInput> & input)
	{
		if (input)
		{
			Ptr<D3D11RenderInput> d3dRenderInput = std::static_pointer_cast<D3D11RenderInput>(input);
			Ptr<D3D11VertexShaderProgram> d3dVS = std::static_pointer_cast<D3D11VertexShaderProgram>(_state.shaders[SHADER_VS]->GetProgram());
			_rawD3DDeviceContext->IASetInputLayout(d3dRenderInput->AcquireRawD3DInputLayout(d3dVS).get());
			_rawD3DDeviceContext->IASetPrimitiveTopology(GetD3DPrimitiveTopology(d3dRenderInput->GetPrimitiveTopology()));

			//Set Vertex Buffers
			std::vector<ID3D11Buffer*> d3dVertexBufferList;
			std::vector<uint32_t> strides;
			std::vector<uint32_t> offsets;
			int32_t index = 0;
			for (auto & vb : d3dRenderInput->GetVerticesBuffers())
			{
				auto d3dVB = std::static_pointer_cast<D3D11RenderBuffer>(vb);
				d3dVertexBufferList.push_back(d3dVB->RawD3DBuffer().get());
				strides.push_back(static_cast<uint32_t>(d3dVB->Desc().elementSize));
				offsets.push_back(static_cast<uint32_t>(input->GetVerticesBytesOffset(index)));

				++index;
			}

			if (_cachedNumVBs > static_cast<int32_t>(d3dVertexBufferList.size()))
			{
				d3dVertexBufferList.resize(_cachedNumVBs);
				strides.resize(_cachedNumVBs);
				offsets.resize(_cachedNumVBs);
			}

			_rawD3DDeviceContext->IASetVertexBuffers(0, static_cast<UINT>(d3dVertexBufferList.size()), &d3dVertexBufferList[0], &strides[0], &offsets[0]);
			_cachedNumVBs = static_cast<int32_t>(d3dRenderInput->GetVerticesBuffers().size());

			//Set Index Buffer
			if (d3dRenderInput->GetIndicesBuffer())
			{
				auto d3dIB = std::static_pointer_cast<D3D11RenderBuffer>(d3dRenderInput->GetIndicesBuffer());
				DXGI_FORMAT ibFmt = DXGI_FORMAT_UNKNOWN;
				if (4 == d3dIB->Desc().elementSize)
					ibFmt = DXGI_FORMAT_R32_UINT;
				else if (2 == d3dIB->Desc().elementSize)
					ibFmt = DXGI_FORMAT_R16_UINT;

				if (DXGI_FORMAT_UNKNOWN == ibFmt)
					ToyGE_ASSERT_FAIL("Index Buffer format error");

				_rawD3DDeviceContext->IASetIndexBuffer(d3dIB->RawD3DBuffer().get(), ibFmt, static_cast<uint32_t>(input->GetIndicesBytesOffset()));
			}
			else
			{
				_rawD3DDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			}
		}
		else
		{
			_rawD3DDeviceContext->IASetInputLayout(nullptr);

			std::vector<ID3D11Buffer*> d3dVertexBufferList(_cachedNumVBs);
			std::vector<uint32_t> strides(_cachedNumVBs);
			std::vector<uint32_t> offsets(_cachedNumVBs);

			_rawD3DDeviceContext->IASetVertexBuffers(0, static_cast<UINT>(d3dVertexBufferList.size()), &d3dVertexBufferList[0], &strides[0], &offsets[0]);

			_rawD3DDeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}
	}

	void D3D11RenderContext::DoSetShaderProgram(const Ptr<ShaderProgram> & program)
	{
		switch (program->Type())
		{
		case SHADER_VS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11VertexShaderProgram>(program);
			_rawD3DDeviceContext->VSSetShader(d3dShader->RawD3DVertexShader().get(), 0, 0);
			break;
		}
		case SHADER_HS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11HullShaderProgram>(program);
			_rawD3DDeviceContext->HSSetShader(d3dShader->RawD3DHullShader().get(), 0, 0);
			break;
		}
		case SHADER_DS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11DomainShaderProgram>(program);
			_rawD3DDeviceContext->DSSetShader(d3dShader->RawD3DDomainShader().get(), 0, 0);
			break;
		}
		case SHADER_GS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11GeometryShaderProgram>(program);
			_rawD3DDeviceContext->GSSetShader(d3dShader->RawD3DGeometryShader().get(), 0, 0);
			break;
		}
		case SHADER_PS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11PixelShaderProgram>(program);
			_rawD3DDeviceContext->PSSetShader(d3dShader->RawD3DPixelShader().get(), 0, 0);
			break;
		}
		case SHADER_CS:
		{
			auto d3dShader = std::static_pointer_cast<D3D11ComputeShaderProgram>(program);
			_rawD3DDeviceContext->CSSetShader(d3dShader->RawD3DComputeShader().get(), 0, 0);
			break;
		}
		default:
			break;
		}
	}

	void D3D11RenderContext::DoResetShaderProgram(ShaderType shaderType)
	{
		switch (shaderType)
		{
		case SHADER_VS:
		{
			_rawD3DDeviceContext->VSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_HS:
		{
			_rawD3DDeviceContext->HSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_DS:
		{
			_rawD3DDeviceContext->DSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_GS:
		{
			_rawD3DDeviceContext->GSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_PS:
		{
			_rawD3DDeviceContext->PSSetShader(nullptr, 0, 0);
			break;
		}
		case SHADER_CS:
		{
			_rawD3DDeviceContext->CSSetShader(nullptr, 0, 0);
			break;
		}
		default:
			break;
		}
	}

	void D3D11RenderContext::DoSetBlendState(const Ptr<BlendState> & state, const std::vector<float> & blendFactors, uint32_t sampleMask)
	{
		if (state)
		{
			const float *pFactors = nullptr;
			if (blendFactors.size() < 4)
				return;
			else
				pFactors = &blendFactors[0];

			Ptr<D3D11BlendState> d3dState = std::static_pointer_cast<D3D11BlendState>(state);
			_rawD3DDeviceContext->OMSetBlendState(d3dState->RawD3DBlendState().get(), pFactors, sampleMask);
		}
		else
			_rawD3DDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	}

	void D3D11RenderContext::DoSetDepthStencilState(const Ptr<DepthStencilState> & state, uint32_t steincilRef)
	{
		if (state)
		{
			Ptr<D3D11DepthStencilState> d3dState = std::static_pointer_cast<D3D11DepthStencilState>(state);
			_rawD3DDeviceContext->OMSetDepthStencilState(d3dState->RawD3DDepthStencilState().get(), steincilRef);
		}
		else
			_rawD3DDeviceContext->OMSetDepthStencilState(nullptr, 0);
	}

	void D3D11RenderContext::DoSetRasterizerState(const Ptr<RasterizerState> & state)
	{
		if (state)
		{
			Ptr<D3D11RasterizerState> d3dState = std::static_pointer_cast<D3D11RasterizerState>(state);
			_rawD3DDeviceContext->RSSetState(d3dState->RawD3DRasterizerState().get());
		}
		else
			_rawD3DDeviceContext->RSSetState(nullptr);
	}

	void D3D11RenderContext::DoSetShaderResources(ShaderType shaderType, const std::vector<ResourceView> & shaderResources, int32_t offset)
	{
		std::vector<ID3D11ShaderResourceView*> d3dSRVList;
		for (auto & srView : shaderResources)
		{
			if (!srView.resource)
			{
				d3dSRVList.push_back(nullptr);
				continue;
			}

			if (RENDER_RESOURCE_TEXTURE == srView.resource->ResourceType())
			{
				Ptr<D3D11Texture> d3dTex = std::static_pointer_cast<D3D11Texture>(srView.resource);
				auto & subDesc = srView.subDesc;

				Ptr<ID3D11ShaderResourceView> pSRV;
				if (!subDesc.textureDesc.bAsCube)
					pSRV = d3dTex->AcquireRawD3DShaderResourceView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.mipLevels,
					subDesc.textureDesc.firstArray,
					subDesc.textureDesc.arraySize,
					srView.formatHint);
				else
					pSRV = d3dTex->AcquireRawD3DShaderResourceView_Cube(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.mipLevels,
					subDesc.textureDesc.firstFaceOffset,
					subDesc.textureDesc.numCubes,
					srView.formatHint);

				d3dSRVList.push_back(pSRV.get());
			}
			else
			{
				Ptr<D3D11RenderBuffer> d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(srView.resource);
				auto & curDesc = srView.subDesc;

				Ptr<ID3D11ShaderResourceView> pSRV;
				pSRV = d3dBuffer->AcquireRawD3DShaderResourceView(curDesc.bufferDesc.firstElement, curDesc.bufferDesc.numElements, srView.formatHint);

				d3dSRVList.push_back(pSRV.get());
			}
		}

		if (d3dSRVList.size() > 0)
			_setSRVFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, static_cast<UINT>(d3dSRVList.size()), &d3dSRVList[0]);
		else
			_setSRVFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::DoSetRTsAndUAVs(const std::vector<ResourceView> & targets, const ResourceView & depthStencil, const std::vector<ResourceView> & uavs)
	{
		std::vector<ID3D11RenderTargetView*> d3dRts;
		GetRawD3DRenderTargets(targets, d3dRts);

		ID3D11DepthStencilView *d3dDepthStencil = GetRawD3DDepthStencil(depthStencil);

		std::vector<ID3D11UnorderedAccessView*> d3dUAVs;
		std::vector<uint32_t> initalCounts;
		for (auto & uavDesc : uavs)
		{
			if (!uavDesc.resource)
			{
				d3dUAVs.push_back(nullptr);
				initalCounts.push_back(0);
				continue;
			}

			if (RENDER_RESOURCE_TEXTURE == uavDesc.resource->ResourceType())
			{
				Ptr<D3D11Texture> d3dTex = std::static_pointer_cast<D3D11Texture>(uavDesc.resource);
				auto & subDesc = uavDesc.subDesc;

				Ptr<ID3D11UnorderedAccessView> pUAV;
				if (!subDesc.textureDesc.bAsCube)
					pUAV = d3dTex->AcquireRawD3DUnorderedAccessView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray,
					subDesc.textureDesc.arraySize,
					uavDesc.formatHint);
				else
					pUAV = d3dTex->AcquireRawD3DUnorderedAccessView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray * 6,
					subDesc.textureDesc.arraySize * 6,
					uavDesc.formatHint);

				d3dUAVs.push_back(pUAV.get());
				initalCounts.push_back(subDesc.textureDesc.uavInitalCounts);
			}
			else
			{
				Ptr<D3D11RenderBuffer> d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(uavDesc.resource);
				auto & curDesc = uavDesc.subDesc;

				Ptr<ID3D11UnorderedAccessView> pUAV;
				pUAV = d3dBuffer->AcquireRawD3DUnorderedAccessView(
					curDesc.bufferDesc.firstElement,
					curDesc.bufferDesc.numElements,
					uavDesc.formatHint,
					curDesc.bufferDesc.uavFlags);

				d3dUAVs.push_back(pUAV.get());
				initalCounts.push_back(curDesc.bufferDesc.uavInitalCounts);
			}
		}

		//uint32_t numRts = static_cast<uint32_t>(d3dRts.size());
		//while (numRts >= 1 && d3dRts[numRts - 1] == nullptr)
		//	--numRts;

		_rawD3DDeviceContext.get()->OMSetRenderTargets(static_cast<uint32_t>(d3dRts.size()), d3dRts.size() > 0 ? &d3dRts[0] : nullptr, d3dDepthStencil);

		while (d3dRts.size() > 0 && d3dRts.back() == nullptr)
			d3dRts.pop_back();

		_rawD3DDeviceContext.get()->OMSetRenderTargetsAndUnorderedAccessViews(
			static_cast<UINT>(d3dRts.size()),
			d3dRts.size() > 0 ? &d3dRts[0] : nullptr,
			d3dDepthStencil,
			static_cast<uint32_t>(d3dRts.size()),
			static_cast<UINT>(d3dUAVs.size()),
			&d3dUAVs[0], &initalCounts[0]);
	}

	void D3D11RenderContext::DoSetUAVs(ShaderType shaderType, const std::vector<ResourceView> & resources, int32_t offset)
	{
		std::vector<ID3D11UnorderedAccessView*> d3dUAVList;
		std::vector<uint32_t> initalCounts;
		for (auto & uavDesc : resources)
		{
			if (!uavDesc.resource)
			{
				d3dUAVList.push_back(nullptr);
				initalCounts.push_back(0);
				continue;
			}

			if (RENDER_RESOURCE_TEXTURE == uavDesc.resource->ResourceType())
			{
				Ptr<D3D11Texture> d3dTex = std::static_pointer_cast<D3D11Texture>(uavDesc.resource);
				auto & subDesc = uavDesc.subDesc;

				Ptr<ID3D11UnorderedAccessView> pUAV;
				if (!subDesc.textureDesc.bAsCube)
					pUAV = d3dTex->AcquireRawD3DUnorderedAccessView(
					  subDesc.textureDesc.firstMipLevel,
					  subDesc.textureDesc.firstArray,
					  subDesc.textureDesc.arraySize,
					  uavDesc.formatHint);
				else
					pUAV = d3dTex->AcquireRawD3DUnorderedAccessView(
					  subDesc.textureDesc.firstMipLevel,
					  subDesc.textureDesc.firstArray * 6,
					  subDesc.textureDesc.arraySize * 6,
					  uavDesc.formatHint);

				d3dUAVList.push_back(pUAV.get());
				initalCounts.push_back(subDesc.textureDesc.uavInitalCounts);
			}
			else
			{
				Ptr<D3D11RenderBuffer> d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(uavDesc.resource);
				auto & curDesc = uavDesc.subDesc;

				Ptr<ID3D11UnorderedAccessView> pUAV;
				pUAV = d3dBuffer->AcquireRawD3DUnorderedAccessView(
					curDesc.bufferDesc.firstElement,
					curDesc.bufferDesc.numElements,
					uavDesc.formatHint,
					curDesc.bufferDesc.uavFlags);

				d3dUAVList.push_back(pUAV.get());
				initalCounts.push_back(curDesc.bufferDesc.uavInitalCounts);
			}

		}

		if (d3dUAVList.size() > 0)
			_setUAVFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, static_cast<UINT>(d3dUAVList.size()), &d3dUAVList[0], &initalCounts[0]);
		else
			_setUAVFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, 0, nullptr, &initalCounts[0]);
	}

	void D3D11RenderContext::DoSetShaderBuffers(ShaderType shaderType, const std::vector<Ptr<RenderBuffer>> & buffers, int32_t offset)
	{
		std::vector<ID3D11Buffer*> d3dBufferList;

		for (auto & buffer : buffers)
		{
			if (!buffer)
			{
				d3dBufferList.push_back(nullptr);
				continue;
			}

			Ptr<D3D11RenderBuffer> d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(buffer);
			d3dBufferList.push_back(d3dBuffer->RawD3DBuffer().get());
		}

		if (d3dBufferList.size() > 0)
			_setBufferFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, static_cast<UINT>(d3dBufferList.size()), &d3dBufferList[0]);
		else
			_setBufferFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::DoSetShaderSamplers(ShaderType shaderType, const std::vector<Ptr<Sampler>> & samplers, int32_t offset)
	{
		std::vector<ID3D11SamplerState*> d3dSamplerList;

		for (auto & sampler : samplers)
		{
			if (!sampler)
			{
				d3dSamplerList.push_back(nullptr);
				continue;
			}

			Ptr<D3D11Sampler> d3dSampler = std::static_pointer_cast<D3D11Sampler>(sampler);
			d3dSamplerList.push_back(d3dSampler->RawD3DSampler().get());
		}

		if (d3dSamplerList.size() > 0)
			_setSamplerFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, static_cast<UINT>(d3dSamplerList.size()), &d3dSamplerList[0]);
		else
			_setSamplerFuncMap[shaderType](_rawD3DDeviceContext.get(), offset, 0, nullptr);
	}

	void D3D11RenderContext::GetRawD3DRenderTargets(const std::vector<ResourceView> & renderTargets, std::vector<ID3D11RenderTargetView*> & outRenderTargets)
	{
		for (auto & rtView : renderTargets)
		{
			if (!rtView.resource)
			{
				outRenderTargets.push_back(nullptr);
				continue;
			}

			Ptr<ID3D11RenderTargetView> d3dRtv;
			if (RENDER_RESOURCE_TEXTURE == rtView.resource->ResourceType())
			{
				auto d3dTexture = std::static_pointer_cast<D3D11Texture>(rtView.resource);
				auto & subDesc = rtView.subDesc;

				if (!rtView.subDesc.textureDesc.bAsCube)
					d3dRtv = d3dTexture->AcquireRawD3DRenderTargetView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray,
					subDesc.textureDesc.arraySize,
					rtView.formatHint);
				else
					d3dRtv = d3dTexture->AcquireRawD3DRenderTargetView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray * 6,
					subDesc.textureDesc.arraySize * 6,
					rtView.formatHint);
			}
			else
			{
				auto d3dBuffer = std::static_pointer_cast<D3D11RenderBuffer>(rtView.resource);
				Ptr<ID3D11RenderTargetView> d3dRtv;
				d3dRtv = d3dBuffer->AcquireRawD3DRenderTargetView(rtView.subDesc.bufferDesc.firstElement, rtView.subDesc.bufferDesc.numElements, rtView.formatHint);
			}

			outRenderTargets.push_back(d3dRtv.get());
		}
	}

	ID3D11DepthStencilView * D3D11RenderContext::GetRawD3DDepthStencil(const ResourceView & dsView)
	{
		Ptr<ID3D11DepthStencilView> d3dDepthStencil;
		if (dsView.resource)
		{
			if (RENDER_RESOURCE_TEXTURE == dsView.resource->ResourceType())
			{
				auto d3dTexture = std::static_pointer_cast<D3D11Texture>(dsView.resource);
				auto & subDesc = dsView.subDesc;
				if (!dsView.subDesc.textureDesc.bAsCube)
					d3dDepthStencil = d3dTexture->AcquireRawD3DDepthStencilView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray,
					subDesc.textureDesc.arraySize,
					dsView.formatHint);
				else
					d3dDepthStencil = d3dTexture->AcquireRawD3DDepthStencilView(
					subDesc.textureDesc.firstMipLevel,
					subDesc.textureDesc.firstArray * 6,
					subDesc.textureDesc.arraySize * 6,
					dsView.formatHint);
			}
			else
			{
				ToyGE_ASSERT_FAIL("Buffer can not be set as depthStencil");
			}
		}

		return d3dDepthStencil.get();
	}
}