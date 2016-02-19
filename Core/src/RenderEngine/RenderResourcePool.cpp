#include "ToyGE\RenderEngine\RenderResourcePool.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	uint64_t TexturePool::GetPoolKey(const PooledTextureDesc & key)
	{
		auto desc = key;
		if (desc.texDesc.mipLevels == 0)
		{
			auto mipx = static_cast<int32_t>(std::ceil(std::log2(static_cast<float>(desc.texDesc.width))));
			auto mipy = static_cast<int32_t>(std::ceil(std::log2(static_cast<float>(desc.texDesc.height))));
			auto mipz = static_cast<int32_t>(std::ceil(std::log2(static_cast<float>(desc.texDesc.depth))));
			desc.texDesc.mipLevels = std::max<int32_t>(std::max<int32_t>(mipx, mipy), mipz);
		}

		auto hashKey = Hash(&desc, sizeof(desc));
		return hashKey;
	}

	PooledRenderResourceReference<Texture> TexturePool::NewResource(const PooledTextureDesc & key)
	{
		auto newTex = Global::GetRenderEngine()->GetRenderFactory()->CreateTexture(key.textype);
		newTex->SetDesc(key.texDesc);
		newTex->Init({});
		return PooledRenderResourceReference<Texture>(new PooledRenderResource<Texture>(newTex));
	}


	uint64_t BufferPool::GetPoolKey(const RenderBufferDesc & key)
	{
		auto hashKey = Hash(&key, sizeof(key));
		return hashKey;
	}

	PooledRenderResourceReference<RenderBuffer> BufferPool::NewResource(const RenderBufferDesc & key)
	{
		auto newBuf = Global::GetRenderEngine()->GetRenderFactory()->CreateBuffer();
		newBuf->SetDesc(key);
		newBuf->Init({});
		return PooledRenderResourceReference<RenderBuffer>(new PooledRenderResource<RenderBuffer>(newBuf));
	}
}