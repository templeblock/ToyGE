#pragma once
#ifndef MATERIALASSET_H
#define MATERIALASSET_H

#include "ToyGE\Kernel\Asset.h"
#include "ToyGE\Math\Math.h"

namespace ToyGE
{
	enum MaterialTextureType : uint32_t
	{
		MAT_TEX_BASECOLOR	= 0UL,
		MAT_TEX_ROUGHNESS	= 1UL,
		MAT_TEX_METALLIC	= 2UL,
		MAT_TEX_BUMP		= 3UL,
		MAT_TEX_OPACITYMASK = 4UL,
		MAT_TEX_EMISSIVE	= 5UL
	};

	namespace MaterialTextureTypeNum
	{
		enum MaterialTextureTypeNumDef : uint32_t
		{
			NUM = MAT_TEX_EMISSIVE + 1UL
		};
	};

	namespace MaxNumTexCoord
	{
		enum MaxNumTexCoordDef : uint32_t
		{
			NUM = 32UL
		};
	}

	struct MaterialAssetTexture
	{
		Ptr<class TextureAsset> texAsset;
		uint8_t texCoordIndex;
	};

	class Material;

	class TOYGE_CORE_API MaterialAsset : public Asset
	{
	public:
		virtual ~MaterialAsset() = default;

		virtual void Load() override;

		virtual void Save() override;

		virtual void Init() override;

		virtual String GetAssetExtension() const override
		{
			return MaterialAsset::GetExtension();
		}

		void SetBaseColor(const float3 & baseColor) { _baseColor = baseColor; }
		const float3 & GetBaseColor() const { return _baseColor; }

		void SetRoughness(float roughtness) { _roughness = roughtness; }
		const float & GetRoughness() const { return _roughness; }

		void SetMetallic(float metallic) { _metallic = metallic; }
		const float & GetMetallic() const { return _metallic; }

		void SetEmissive(const float3 & emissive) { _emissive = emissive; }
		const float3 & GetEmissive() const { return _emissive; }

		void SetTranslucent(bool bTranslucent) { _bTranslucent = bTranslucent; }
		bool IsTranslucent() const { return _bTranslucent; }

		void SetOpcacity(float opacity) { _opacity = opacity; }
		float GetOpacity() const { return _opacity; }

		void SetRefraction(bool bRefraction) { _bRefraction = bRefraction; }
		bool IsRefraction() const { return _bRefraction; }

		void SetRefractionIndex(float refractionIndex) { _refractionIndex = refractionIndex; }
		float GetRefractionIndex() const { return _refractionIndex; }

		void SetDualFace(bool bDualFace) { _bDualFace = bDualFace; }
		bool IsDualFace() const { return _bDualFace; }

		void SetPOM(bool bPOM) { _bPOM = bPOM; }
		bool IsPOM() const { return _bPOM; }

		void SetPOMScale(float pomScale) { _pomScale = pomScale; }
		float GetPOMScale() const { return _pomScale; }

		void SetSubsurfaceScattering(bool bSubsurfaceScattering) { _bSubsurfaceScattering = bSubsurfaceScattering; }
		bool IsSubsurfaceScattering() const { return _bSubsurfaceScattering; }

		void AddTexture(MaterialTextureType matTexType, const Ptr<class TextureAsset> & texAsset, uint8_t texCoordIndex)
		{ 
			_textures[matTexType].push_back( { texAsset, texCoordIndex } );
		}
		const std::vector<MaterialAssetTexture> & GetTexture(MaterialTextureType matTexType) { return _textures[matTexType]; }

		CLASS_GET(Material, Ptr<Material>, _material);

		static Ptr<MaterialAsset> New()
		{
			return std::shared_ptr<MaterialAsset>(new MaterialAsset);
		}

		static const String & GetExtension()
		{
			const static String ex = ".tmat";
			return ex;
		}

	private:
		float3	_baseColor = 1.0f;
		float	_roughness = 1.0f;
		float	_metallic = 0.0f;
		float3	_emissive = 0.0f;
		bool	_bTranslucent = false;
		float	_opacity = 1.0f;
		bool	_bRefraction = false;
		float	_refractionIndex = 1.0f;
		bool	_bDualFace = false;
		bool	_bPOM = false;
		float	_pomScale = 0.0f;
		bool	_bSubsurfaceScattering = false;

		std::array<std::vector<MaterialAssetTexture>, MaterialTextureTypeNum::NUM> _textures;

		Ptr<Material> _material;

		void UpdateFromMaterial();
	};
}

#endif