#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Math\Math.h"
#include "ToyGE\Kernel\IOHelper.h"

namespace ToyGE
{
	class RenderEffect;
	class Texture;

	enum MaterialTextureType : uint32_t
	{
		MATERIAL_TEXTURE_BASECOLOR = 0UL,
		MATERIAL_TEXTURE_SPECULAR = 1UL,
		MATERIAL_TEXTURE_SHININESS = 2UL,
		MATERIAL_TEXTURE_ROUGHNESS = 3UL,
		MATERIAL_TEXTURE_BUMP = 4UL,
		MATERIAL_TEXTURE_OPACITYMASK = 5UL,
		MATERIAL_TEXTURE_EMISSIVE = 6UL
	};

	namespace MaterialTextureTypeNum
	{
		enum MaterialTextureTypeNumDef : uint32_t
		{
			value = MATERIAL_TEXTURE_EMISSIVE + 1
		};
	}

	struct MaterialTexture
	{
		WString filePath;
		int8_t texCoordIndex;
	};

	class TOYGE_CORE_API Material : public std::enable_shared_from_this<Material>
	{
		friend class RenderMaterial;

	public:
		static Ptr<Material> LoadBin(const Ptr<Reader> & reader, const WString & basePath);

		Material();

		void SaveBin(const Ptr<Writer> & writer, const WString & skipPrefixPath);

		void SetName(const WString & name)
		{
			_name = name;
		}

		const WString & Name() const
		{
			return _name;
		}

		void AddTexture(MaterialTextureType texType, const WString & filePath, int8_t texCoordIndex)
		{
			_textures[texType].push_back({ filePath, texCoordIndex });
		}

		int32_t NumTextures(MaterialTextureType texType) const
		{
			auto itr = _textures.find(texType);
			if (itr != _textures.end())
				return static_cast<int32_t>(itr->second.size());
			else
				return 0;
		}

		WString GetTexture(MaterialTextureType texType, int32_t index) const
		{
			auto itr = _textures.find(texType);
			if (itr != _textures.end())
			{
				return itr->second[index].filePath;
			}
			else
				return WString();
		}

		CLASS_GET(BaseColor, float3, _baseColor);
		CLASS_SET(BaseColor, float3, _baseColor);

		CLASS_GET(Roughness, float, _roughness);
		CLASS_SET(Roughness, float, _roughness);

		CLASS_GET(Metallic, float, _metallic);
		CLASS_SET(Metallic, float, _metallic);

		CLASS_GET(Emissive, float3, _emissive);
		CLASS_SET(Emissive, float3, _emissive);

		const Ptr<RenderMaterial> & InitRenderData();

		const Ptr<RenderMaterial> & GetRender() const
		{
			return _renderData;
		}

		const Ptr<RenderMaterial> & AcquireRender()
		{
			if (!_renderData)
				return InitRenderData();
			else
				return GetRender();
		}

		uint32_t GetTypeFlags() const;

		void SetTranslucent(bool bTranslucent)
		{
			_bTranslucent = bTranslucent;
		}

		bool IsTranslucent() const
		{
			return _bTranslucent;
		}

		void SetOpacity(float opacity)
		{
			_opacity = std::min<float>(1.0f, std::max<float>(opacity, 0.0f));
		}

		float GetOpacity() const
		{
			return _opacity;
		}

		void SetRefraction(bool bRefraction)
		{
			_bRefraction = bRefraction;
		}

		bool IsRefraction() const
		{
			return _bRefraction;
		}

		void SetRefractionIndex(float index)
		{
			_refractionIndex = index;
		}

		float GetRefractionIndex() const
		{
			return _refractionIndex;
		}

		void SetDualFace(bool bDualFace)
		{
			_bDualFace = bDualFace;
		}

		bool IsDualFace() const
		{
			return _bDualFace;
		}

		void SetPOM(bool bPOM)
		{
			_bPOM = bPOM;
		}

		bool IsPOM() const
		{
			return _bPOM;
		}

		void SetPOMScale(float scale)
		{
			_pomScale = scale;
		}

		float GetPOMScale() const
		{
			return _pomScale;
		}

		void SetSubSurfaceScattering(bool bSubSurface)
		{
			_bSubSurfaceScattering = bSubSurface;
		}

		bool IsSubSurfaceScattering() const
		{
			return _bSubSurfaceScattering;
		}

		void BindMacros(const Ptr<RenderEffect> & effect);

		void BindParams(const Ptr<RenderEffect> & effect);

	private:
		WString _name;
		std::map<MaterialTextureType, std::vector<MaterialTexture> > _textures;
		float3 _baseColor;
		float _roughness;
		float _metallic;
		float3 _emissive;
		bool _bTranslucent;
		float _opacity;
		bool _bRefraction;
		float _refractionIndex;
		bool _bDualFace;
		bool _bPOM;
		float _pomScale;
		bool _bSubSurfaceScattering;
		Ptr<RenderMaterial> _renderData;
	};

	class TOYGE_CORE_API RenderMaterial
	{
	public:
		RenderMaterial(const Ptr<Material> & material);

		int32_t NumTextures(MaterialTextureType texType) const
		{
			auto itr = _textures.find(texType);
			if (itr != _textures.end())
				return static_cast<int32_t>(itr->second.size());
			else
				return 0;
		}

		Ptr<Texture> GetTexture(MaterialTextureType texType, int32_t index) const
		{
			auto itr = _textures.find(texType);
			if (itr != _textures.end())
			{
				return itr->second[index];
			}
			else
			{
				return Ptr<Texture>();
			}
		}

	private:
		std::map < MaterialTextureType, std::vector<Ptr<Texture>> >  _textures;
	};
}

#endif