#pragma once
#ifndef RENDERRESOURCE_H
#define RENDERRESOURCE_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"

#include <boost\noncopyable.hpp>

namespace ToyGE
{
	enum class RenderResourceBoundType
	{
		RRBT_SRV,
		RRBT_UAV,
		RRBT_RTV,
		RRBT_DSV,
		RRBT_SAMPLER,
		RRBT_STATE,
		RRBT_VB,
		RRBT_IB,
		RRBT_CB
	};

	struct RenderResourceBoundState
	{
		RenderResourceBoundType boundType;
		ShaderType boundShaderType;
		int32_t boundIndex;
	};

	enum class RenderResourceType
	{
		RRT_TEXTURE,
		RRT_BUFFER,
		RRT_SHADER,
		RRT_SAMPLER,
		RRT_BLENDSTATE,
		RRT_RASTERIZERSTATE,
		RRT_DEPTHSTENCILSTATE,
		RRT_VERTEXINPUTLAYOUT
	};

	class TOYGE_CORE_API RenderResource : public StaticCastable, public boost::noncopyable
	{
	public:
		RenderResource(RenderResourceType type);

		virtual ~RenderResource();

		// Init hardware resource
		virtual void Init();

		// Release hardware resource
		virtual void Release();
		
		bool IsBound() const
		{
			return _bBound;
		}

		bool IsBoundAsInput() const
		{
			return IsBound() &&
				(	_boundState.boundType == RenderResourceBoundType::RRBT_SRV ||
					_boundState.boundType == RenderResourceBoundType::RRBT_SAMPLER ||
					_boundState.boundType == RenderResourceBoundType::RRBT_STATE || 
					_boundState.boundType == RenderResourceBoundType::RRBT_VB || 
					_boundState.boundType == RenderResourceBoundType::RRBT_IB || 
					_boundState.boundType == RenderResourceBoundType::RRBT_CB);
		}

		bool IsBoundAsOutput() const
		{
			return IsBound() && !IsBoundAsInput();
		}

		void SetBound(RenderResourceBoundType boundType, ShaderType boundShaderType, int32_t boundIndex);

		void SetBound(const RenderResourceBoundState & boundState);

		void ResetBound()
		{
			_bBound = false;
		}

		const RenderResourceBoundState GetBoundState() const
		{
			return _boundState;
		}

		CLASS_GET(ResourceType, RenderResourceType, _resourceType);

		static std::list<std::weak_ptr<RenderResource>> & GetRenderResourceList()
		{
			static std::list<std::weak_ptr<RenderResource>> _renderResources;
			return _renderResources;
		}

		void Register()
		{
			GetRenderResourceList().push_back(Cast<RenderResource>());
		}

		static void ClearExpiredResources()
		{
			for (auto itr = GetRenderResourceList().begin(); itr != GetRenderResourceList().end(); ++itr)
			{
				if (itr->expired())
				{
					itr = GetRenderResourceList().erase(itr);
					if (itr == GetRenderResourceList().end())
						break;
				}
			}
		}

		bool IsInit() const
		{
			return _bInit;
		}

	protected:
		RenderResourceType _resourceType;
		bool _bBound = false;
		bool _bInit = false;
		RenderResourceBoundState _boundState;
	};


	class TOYGE_CORE_API ResourceView : public StaticCastable
	{
	public:
		RenderFormat viewFormat;

		ResourceView()
			: viewFormat(RENDER_FORMAT_UNDEFINED)
		{

		}

		virtual ~ResourceView() = default;

		virtual void Release() {}

		void SetResource(const Ptr<RenderResource> & resource)
		{
			_resource = resource;
		}

		Ptr<RenderResource> GetResource() const
		{
			return _resource.lock();
		}

	protected:
		std::weak_ptr<RenderResource> _resource;
	};

	class TOYGE_CORE_API ShaderResourceView : public ResourceView
	{
	public:
		virtual ~ShaderResourceView() = default;
	};

	class TOYGE_CORE_API UnorderedAccessView : public ResourceView
	{
	public:
		virtual ~UnorderedAccessView() = default;
	};

	class TOYGE_CORE_API RenderTargetView : public ResourceView
	{
	public:
		virtual ~RenderTargetView() = default;
	};

	class TOYGE_CORE_API DepthStencilView : public ResourceView
	{
	public:
		virtual ~DepthStencilView() = default;
	};
}

#endif