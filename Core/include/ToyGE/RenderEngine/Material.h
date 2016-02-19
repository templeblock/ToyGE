#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include "ToyGE\Kernel\MaterialAsset.h"

namespace ToyGE
{
	class Texture;
	class Shader;

	struct MaterialTexture
	{
		Ptr<Texture> texture;
		uint8_t texCoordIndex;
	};

	class TOYGE_CORE_API Material : public std::enable_shared_from_this<Material>
	{
		friend class MaterialAsset;
	public:
		void Init();

		void SetBaseColor(const float3 & baseColor) 
		{ 
			_baseColor = baseColor; 
			_bDirty = true; 
		}
		const float3 & GetBaseColor() const { return _baseColor; }

		void SetRoughness(float roughtness) 
		{ 
			_roughness = roughtness; 
			_bDirty = true; 
		}
		const float & GetRoughness() const { return _roughness; }

		void SetMetallic(float metallic) 
		{ 
			_metallic = metallic;
			_bDirty = true; 
		}
		const float & GetMetallic() const { return _metallic; }

		void SetEmissive(const float3 & emissive) 
		{ 
			_emissive = emissive; 
			_bDirty = true;
		}
		const float3 & GetEmissive() const { return _emissive; }

		void SetTranslucent(bool bTranslucent) 
		{
			_bTranslucent = bTranslucent; 
			_bDirty = true;
		}
		bool IsTranslucent() const { return _bTranslucent; }

		void SetOpcacity(float opacity) 
		{ 
			_opacity = opacity; 
			_bDirty = true;
		}
		float GetOpacity() const { return _opacity; }

		void SetRefraction(bool bRefraction) 
		{ 
			_bRefraction = bRefraction; 
			_bDirty = true;
		}
		bool IsRefraction() const { return _bRefraction; }

		void SetRefractionIndex(float refractionIndex) 
		{ 
			_refractionIndex = refractionIndex; 
			_bDirty = true;
		}
		float GetRefractionIndex() const { return _refractionIndex; }

		void SetDualFace(bool bDualFace) 
		{ 
			_bDualFace = bDualFace; 
			_bDirty = true;
		}
		bool IsDualFace() const { return _bDualFace; }

		void SetPOM(bool bPOM) 
		{ 
			_bPOM = bPOM; 
			_bDirty = true;
		}
		bool IsPOM() const { return _bPOM; }

		void SetPOMScale(float pomScale) 
		{ 
			_pomScale = pomScale; 
			_bDirty = true;
		}
		float GetPOMScale() const { return _pomScale; }

		void SetSubsurfaceScattering(bool bSubsurfaceScattering) 
		{ 
			_bSubsurfaceScattering = bSubsurfaceScattering; 
			_bDirty = true;
		}
		bool IsSubsurfaceScattering() const { return _bSubsurfaceScattering; }

		void AddTexture(MaterialTextureType matTexType, const Ptr<Texture> & texture, uint8_t texCoordIndex) 
		{ 
			_textures[matTexType].push_back({ texture, texCoordIndex });
			_bDirty = true;
		}
		const std::vector<MaterialTexture> & GetTexture(MaterialTextureType matTexType) { return _textures[matTexType]; }

		void SetAsset(const Ptr<MaterialAsset> & asset)
		{
			if (_asset.lock() != asset)
			{
				_asset = asset;
				_bDirty = true;
			}
		}
		Ptr<class MaterialAsset> GetAsset() const
		{
			return _asset.lock();
		}

		void BindMacros(std::map<String, String> & outMacros);

		void BindShaderParams(const Ptr<Shader> & shader);

		void BindDepthMacros(std::map<String, String> & outMacros);

		void BindDepthShaderParams(const Ptr<Shader> & shader);

		CLASS_SET(Dirty, bool, _bDirty);
		bool IsDirty() const
		{
			return _bDirty;
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

		std::array<std::vector<MaterialTexture>, MaterialTextureTypeNum::NUM> _textures;

		std::weak_ptr<MaterialAsset> _asset;
		bool _bDirty = false;
	};
}

#endif