#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include "ToyGE\RenderEngine\RenderResource.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	class Image;

	/**
	 * Texture view
	 */
	class TOYGE_CORE_API TextureResourceView
	{
	public:
		int32_t firstArray = 0; // if srv if cube, firstArray is offset of first cube face
		int32_t numArrays = 0; // if srv if cube, numArrays is number of cubes

		virtual ~TextureResourceView() = default;
	};

	class TOYGE_CORE_API TextureShaderResourceView : public ShaderResourceView, public TextureResourceView
	{
	public:
		bool bCube = false;
		int32_t firstMip;
		int32_t numMips;

		virtual ~TextureShaderResourceView() = default;
	};

	class TOYGE_CORE_API TextureUnorderedAccessView : public UnorderedAccessView, public TextureResourceView
	{
	public:
		int32_t mipLevel = 0;

		virtual ~TextureUnorderedAccessView() = default;
	};

	class TOYGE_CORE_API TextureRenderTargetView : public RenderTargetView, public TextureResourceView
	{
	public:
		int32_t mipLevel = 0;

		virtual ~TextureRenderTargetView() = default;
	};

	class TOYGE_CORE_API TextureDepthStencilView : public DepthStencilView, public TextureResourceView
	{
	public:
		int32_t mipLevel = 0;

		virtual ~TextureDepthStencilView() = default;
	};

	/**
	 * Texture
	 */
	enum TextureType : uint32_t
	{
		TEXTURE_UNDEFINED,
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D,
		//TEXTURE_CUBE
	};

	struct TextureDesc
	{
		RenderFormat format = RENDER_FORMAT_UNDEFINED;
		int32_t width = 1;
		int32_t height = 1;
		int32_t depth = 1;
		int32_t arraySize = 1;
		int32_t mipLevels = 1;
		int32_t sampleCount = 1;
		int32_t sampleQuality = 0;
		uint32_t bindFlag = 0;
		uint32_t cpuAccess = 0;
		uint32_t bCube = 0;
	};

	class TOYGE_CORE_API Texture : public RenderResource
	{
		friend class TextureAsset;
	public:
		Texture(TextureType type) : RenderResource(RenderResourceType::RRT_TEXTURE), _type(type) {};

		virtual ~Texture() = default;

		virtual void Init(const std::vector<RenderDataDesc> & initDataList);

		virtual void Init() override { Init({}); }

		virtual void Release() override;

		CLASS_GET(Type, TextureType, _type);

		void SetDesc(const TextureDesc & desc)
		{
			_desc = desc;
			_bDirty = true;
			_bInit = false;
		}
		CLASS_GET(Desc, TextureDesc, _desc);

		const int3 & GetMipSize(int32_t mipLevel) const
		{
			return _mipsSize[mipLevel];
		}

		//Ptr<Image> CreateImage(bool bWithMipMap) const;

		float4 GetTexSize() const
		{
			float w = static_cast<float>(_desc.width);
			float h = static_cast<float>(_desc.height);
			return float4(w, h, 1.0f / w, 1.0f / h);
		}

		Ptr<Texture> CreateMips() const;

		int32_t GetDataSize() const;

		bool Dump(void * outDumpBuffer, std::vector<RenderDataDesc> & outDataDescs);

		virtual bool GenerateMips() = 0;

		virtual bool CopyTo(
			const Ptr<Texture> & dst,
			int32_t dstMipLevel, int32_t dstArrayIndex, int32_t xOffset, int32_t yOffset, int32_t zOffset,
			int32_t srcMipLevel, int32_t srcArrayIndex, const std::shared_ptr<Box> & srcBox = std::shared_ptr<Box>()) const = 0;

		virtual RenderDataDesc Map(MapType mapFlag, int32_t mipLevel, int32_t arrayIndex) = 0;

		virtual void UnMap() = 0;

		/**
		 * @param	firstMip is the most detailed miplevel
		 * @param	numMips is the number of miplevels, 0 means max levels
		 * @param	firstArray is the first array of texture if bCube is false, and the offset of first cube face if bCube is true
		 * @param	numArrays is the number of arrays of texture if bCube is false, and the number of cubes if bCube is true, 0 means max arrays
		 * @param	bCube is true if this view is for cube or false for not
		 * @param	viewFormat is the format of view, if viewFormat is undefined this view will be created by using this texture's format
		 */
		Ptr<TextureShaderResourceView> GetShaderResourceView(int32_t firstMip = 0, int32_t numMips = 0, int32_t firstArray = 0, int32_t numArrays = 0, bool bCube = false, RenderFormat viewFormat = RENDER_FORMAT_UNDEFINED);

		Ptr<TextureUnorderedAccessView> GetUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat = RENDER_FORMAT_UNDEFINED);

		Ptr<TextureRenderTargetView> GetRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat = RENDER_FORMAT_UNDEFINED);

		Ptr<TextureDepthStencilView> GetDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat = RENDER_FORMAT_UNDEFINED);

		void SetAsset(const Ptr<class TextureAsset> & asset)
		{
			if (_asset.lock() != asset)
			{
				_asset = asset;
				_bDirty = true;
			}
		}
		Ptr<class TextureAsset> GetAsset() const
		{
			return _asset.lock();
		}

		void SetDirty(bool bDirty)
		{
			_bDirty = bDirty;
		}
		bool IsDirty() const
		{
			return _bDirty;
		}

	protected:
		TextureType _type;
		TextureDesc _desc;
		std::vector<int3> _mipsSize;
		std::weak_ptr<class TextureAsset> _asset;
		bool _bDirty = false;

		// Cache views
		std::vector<Ptr<TextureShaderResourceView>>		_srvCache;
		std::vector<Ptr<TextureUnorderedAccessView>>	_uavCache;
		std::vector<Ptr<TextureRenderTargetView>>		_rtvCache;
		std::vector<Ptr<TextureDepthStencilView>>		_dsvCache;

		void InitMipsSize();

		virtual Ptr<TextureShaderResourceView> CreateShaderResourceView(int32_t firstMip, int32_t numMips, int32_t firstArray, int32_t numArrays, bool bCube, RenderFormat viewFormat) = 0;

		virtual Ptr<TextureUnorderedAccessView> CreateUnorderedAccessView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) = 0;

		virtual Ptr<TextureRenderTargetView> CreateRenderTargetView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) = 0;

		virtual Ptr<TextureDepthStencilView> CreateDepthStencilView(int32_t mipLevel, int32_t firstArray, int32_t numArrays, RenderFormat viewFormat) = 0;
	};
}

#endif