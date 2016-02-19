#pragma once
#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

#include "ToyGE\Kernel\Asset.h"
#include "ToyGE\RenderEngine\Texture.h"

namespace ToyGE
{
	class TOYGE_CORE_API TextureAsset : public Asset
	{
	public:
		virtual ~TextureAsset() = default;

		virtual void Load() override;

		virtual void Save() override;

		virtual void Init() override;

		virtual String GetAssetExtension() const override
		{
			return TextureAsset::GetExtension();
		}

		static const String & GetExtension()
		{
			static const String ex = ".dds";
			return ex;
		}

		static Ptr<TextureAsset> New()
		{
			return std::shared_ptr<TextureAsset>(new TextureAsset);
		}

		CLASS_SET(Type, TextureType, _type);
		CLASS_GET(Type, TextureType, _type);

		CLASS_SET(Format, RenderFormat, _format);
		CLASS_GET(Format, RenderFormat, _format);

		CLASS_SET(Width, int32_t, _width);
		CLASS_GET(Width, int32_t, _width);

		CLASS_SET(Height, int32_t, _height);
		CLASS_GET(Height, int32_t, _height);

		CLASS_SET(Depth, int32_t, _depth);
		CLASS_GET(Depth, int32_t, _depth);

		CLASS_SET(ArraySize, int32_t, _arraySize);
		CLASS_GET(ArraySize, int32_t, _arraySize);

		CLASS_SET(MipLevels, int32_t, _mipLevels);
		CLASS_GET(MipLevels, int32_t, _mipLevels);

		void SetCube(bool bCube) { _bCube = bCube; }
		bool IsCube() const
		{
			return _bCube;
		}

		void SetDataDescs(const std::vector<RenderDataDesc> & descs)
		{
			_dataDescs = descs;
			_bDirty = true;
		}

		const std::vector<RenderDataDesc> & GetDataDescs() const
		{
			return _dataDescs;
		}

		void SetData(const std::shared_ptr<uint8_t> & data)
		{
			_data = data;
			_bDirty = true;
		}

		std::shared_ptr<const uint8_t> GetData() const
		{
			return _data;
		}

		CLASS_SET(Texture, Ptr<Texture>, _texture);
		CLASS_GET(Texture, Ptr<Texture>, _texture);

		void Compress(RenderFormat format);

		void Decompress(RenderFormat format);

	private:
		TextureType _type;
		bool _bCube = false;
		int32_t _width;
		int32_t _height;
		int32_t _depth;
		int32_t _arraySize;
		int32_t _mipLevels;
		RenderFormat _format;
		std::vector<RenderDataDesc> _dataDescs;
		std::shared_ptr<uint8_t> _data;

		Ptr<Texture> _texture;

		void UpdateFromTexture();
	};
}

#endif