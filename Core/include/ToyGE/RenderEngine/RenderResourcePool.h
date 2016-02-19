#pragma once
#ifndef RENDERRESOURCEPOOL_H
#define RENDERRESOURCEPOOL_H

#include "ToyGE\Kernel\Singleton.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\RenderEngine\RenderBuffer.h"

namespace ToyGE
{
	template <typename ResourceType>
	class PooledRenderResource
	{
		template <typename ResourceFindKey, typename ResourcePoolKey, typename ResourceType, int32_t maxUnusedCount>
		friend class RenderResourcePool;

		template <typename ResourceType>
		friend class PooledRenderResourceReference;

	public:
		PooledRenderResource(const Ptr<ResourceType> & resource)
		{
			_bInUse = false;
			_unusedCount = 0;
			_resource = resource;
		}

		operator bool() const
		{
			return !!_resource;
		}

		const Ptr<ResourceType> & Get() const
		{
			return _resource;
		}

	protected:
		bool _bInUse;
		int32_t _unusedCount;
		Ptr<ResourceType> _resource;
	};

	template <typename ResourceType>
	class PooledRenderResourceReference
	{
		template <typename ResourceFindKey, typename ResourcePoolKey, typename ResourceType, int32_t maxUnusedCount>
		friend class RenderResourcePool;
		template <typename ResourceType>
		friend class PooledRenderResourceReference;

	public:
		PooledRenderResourceReference()
		{
			_resource = nullptr;
			_referenceCount = nullptr;
		}

		PooledRenderResourceReference(PooledRenderResource<ResourceType> * pooledResource)
			: PooledRenderResourceReference()
		{
			if (pooledResource)
			{
				_resource = pooledResource;
				_referenceCount = new int32_t(1);
			}
		}

		PooledRenderResourceReference(const PooledRenderResourceReference<ResourceType> & rhs)
			: PooledRenderResourceReference()
		{
			_resource = rhs._resource;
			_referenceCount = rhs._referenceCount;

			if(_referenceCount)
				++(*_referenceCount);
		}

		template <typename RhsResourceType>
		PooledRenderResourceReference(const PooledRenderResourceReference<RhsResourceType> & rhs)
			: PooledRenderResourceReference()
		{
			if (rhs)
			{
				_resource = static_cast<PooledRenderResource<ResourceType>*>(rhs._resource);
				_referenceCount = rhs._referenceCount;

				++(*_referenceCount);
			}
		}

		~PooledRenderResourceReference()
		{
			if (_referenceCount)
			{
				--(*_referenceCount);
				if (*_referenceCount == 0)
				{
					delete _resource;
					_resource = nullptr;
					delete _referenceCount;
					_referenceCount = nullptr;
				}
				else if (*_referenceCount == 1)
				{
					_resource->_bInUse = false;
					_resource->_unusedCount = 0;
				}
			}
		}

		void swap(PooledRenderResourceReference<ResourceType> & rhs)
		{
			std::swap(_resource, rhs._resource);
			std::swap(_referenceCount, rhs._referenceCount);
		}

		PooledRenderResourceReference<ResourceType> & operator=(const PooledRenderResourceReference<ResourceType> & rhs)
		{
			PooledRenderResourceReference<ResourceType> tmp(rhs);
			this->swap(tmp);
			return *this;
		}

		template <class RhsResourceType>
		PooledRenderResourceReference<ResourceType> & operator=(const PooledRenderResourceReference<RhsResourceType> & rhs)
		{
			PooledRenderResourceReference<ResourceType> tmp(rhs);
			this->swap(tmp);
			return *this;
		}

		operator bool() const
		{
			return !!_resource;
		}

		PooledRenderResource<ResourceType> * operator->() const
		{
			return _resource;
		}

		PooledRenderResource<ResourceType> * Get() const
		{
			return _resource;
		}

		template <class DstResourceType>
		PooledRenderResourceReference<DstResourceType> Cast()
		{
			return PooledRenderResourceReference<DstResourceType>(*this);
		}


	protected:
		PooledRenderResource<ResourceType> * _resource;
		int32_t * _referenceCount;
	};

	template <typename ResourceFindKey, typename ResourcePoolKey, typename ResourceType, int32_t maxUnusedCount>
	class RenderResourcePool
	{
	public:
		virtual ~RenderResourcePool() = default;

		PooledRenderResourceReference<ResourceType> FindFree(const ResourceFindKey & key)
		{
			auto poolKey = GetPoolKey(key);
			auto & resList = _pooledResources[poolKey];

			PooledRenderResourceReference<ResourceType> resultRes;

			for (auto & res : resList)
			{
				if (!res->_bInUse)
				{
					resultRes = res;
					break;
				}
			}

			if (!resultRes)
			{
				resultRes = NewResource(key);
				resList.push_back(resultRes);
			}

			resultRes->_bInUse = true;
			resultRes->_unusedCount = 0;

			return resultRes;
		}

		void Tick()
		{
			for (auto & resListPair : _pooledResources)
			{
				for (auto itr = resListPair.second.begin(); itr != resListPair.second.end(); )
				{
					++(itr->Get()->_unusedCount);
					if ( !itr->Get()->_bInUse && itr->Get()->_unusedCount > maxUnusedCount )
						itr = resListPair.second.erase(itr);
					else
						++itr;
				}
			}
		}


	protected:
		std::map<ResourcePoolKey, std::list<PooledRenderResourceReference<ResourceType>>> _pooledResources;

		virtual ResourcePoolKey GetPoolKey(const ResourceFindKey & key) = 0;

		virtual PooledRenderResourceReference<ResourceType> NewResource(const ResourceFindKey & key) = 0;
	};

	struct PooledTextureDesc
	{
		TextureType textype;
		TextureDesc texDesc;
	};


	template <>
	class PooledRenderResource<Texture> : public PooledRenderResource<RenderResource>
	{
	public:
		//using PooledRenderResource<RenderResource>::PooledRenderResource;
		PooledRenderResource(const Ptr<Texture> & resource)
			: PooledRenderResource<RenderResource>(resource)
		{
		}
	};

	using PooledTextureRef = PooledRenderResourceReference<Texture>;

	class TOYGE_CORE_API TexturePool : public RenderResourcePool<PooledTextureDesc, uint64_t, Texture, 3>, public Singleton<TexturePool>
	{
	protected:
		uint64_t GetPoolKey(const PooledTextureDesc & key) override;

		PooledRenderResourceReference<Texture> NewResource(const PooledTextureDesc & key) override;
	private:
	};


	template <>
	class PooledRenderResource<RenderBuffer> : public PooledRenderResource<RenderResource>
	{
	public:
		//using PooledRenderResource<RenderResource>::PooledRenderResource;
		PooledRenderResource(const Ptr<RenderBuffer> & resource)
			: PooledRenderResource<RenderResource>(resource)
		{
		}
	};

	using PooledBufferRef = PooledRenderResourceReference<RenderBuffer>;

	class TOYGE_CORE_API BufferPool : public RenderResourcePool<RenderBufferDesc, uint64_t, RenderBuffer, 3>, public Singleton<BufferPool>
	{
	protected:
		uint64_t GetPoolKey(const RenderBufferDesc & key) override;

		PooledRenderResourceReference<RenderBuffer> NewResource(const RenderBufferDesc & key) override;
	private:
	};
}

#endif
