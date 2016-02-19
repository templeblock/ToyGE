#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\Shader.h"
#include "ToyGE\RenderEngine\Texture.h"
#include "ToyGE\Kernel\TextureAsset.h"

namespace ToyGE
{

	void Material::Init()
	{

	}

	void Material::BindMacros(std::map<String, String> & outMacros)
	{
		static const std::map<MaterialTextureType, String> matTexMacrosMap =
		{
			{ MAT_TEX_BASECOLOR,	"MAT_TEX_BASECOLOR" },
			{ MAT_TEX_ROUGHNESS,	"MAT_TEX_ROUGHNESS" },
			{ MAT_TEX_METALLIC,		"MAT_TEX_METALLIC" },
			{ MAT_TEX_BUMP,			"MAT_TEX_BUMP" },
			{ MAT_TEX_OPACITYMASK,	"MAT_TEX_OPACITYMASK" },
			{ MAT_TEX_EMISSIVE,		"MAT_TEX_EMISSIVE" }
		};

		for (auto & macroPair : matTexMacrosMap)
		{
			if (GetTexture(macroPair.first).size() > 0)
			{
				outMacros[macroPair.second] = "1";
				outMacros[macroPair.second + "_TEXCOORDINDEX"] = std::to_string(GetTexture(macroPair.first)[0].texCoordIndex);
			}
			else
			{
				outMacros[macroPair.second] = "0";
			}
		}

		if(IsPOM())
			outMacros["MAT_POM"] = "1";
		else
			outMacros["MAT_POM"] = "0";
	}

	void Material::BindShaderParams(const Ptr<Shader> & shader)
	{
		static const std::map<MaterialTextureType, String> matTexVarsMap =
		{
			{ MAT_TEX_BASECOLOR,	"baseColorTex" },
			{ MAT_TEX_ROUGHNESS,	"roughnessTex" },
			{ MAT_TEX_METALLIC,		"metallicTex" },
			{ MAT_TEX_BUMP,			"bumpTex" },
			{ MAT_TEX_OPACITYMASK,	"opacityMaskTex" },
			{ MAT_TEX_EMISSIVE,		"emissiveTex" }
		};

		for (auto & varPair : matTexVarsMap)
		{
			if (GetTexture(varPair.first).size() > 0)
			{
				auto tex = GetTexture(varPair.first)[0].texture;
				shader->SetSRV(varPair.second, tex->GetShaderResourceView());
			}
		}

		shader->SetScalar("matBaseColor", _baseColor);
		shader->SetScalar("matRoughness", _roughness);
		shader->SetScalar("matMetallic", _metallic);
		shader->SetScalar("matEmissive", _emissive);
		//shader->SetScalar("matPomScale", _pomScale);
		shader->SetScalar("matOpacity", _opacity);
		shader->SetScalar("matRefractionIndex", _refractionIndex);
	}

	void Material::BindDepthMacros(std::map<String, String> & outMacros)
	{
		static const std::map<MaterialTextureType, String> matTexMacrosMap =
		{
			{ MAT_TEX_OPACITYMASK,	"MAT_TEX_OPACITYMASK" }
		};

		for (auto & macroPair : matTexMacrosMap)
		{
			if (GetTexture(macroPair.first).size() > 0)
			{
				outMacros[macroPair.second] = "1";
				outMacros[macroPair.second + "_TEXCOORDINDEX"] = std::to_string(GetTexture(macroPair.first)[0].texCoordIndex);
			}
			else
			{
				outMacros[macroPair.second] = "0";
			}
		}
	}

	void Material::BindDepthShaderParams(const Ptr<Shader> & shader)
	{
		static const std::map<MaterialTextureType, String> matTexVarsMap =
		{
			{ MAT_TEX_OPACITYMASK,	"opacityMaskTex" }
		};

		for (auto & varPair : matTexVarsMap)
		{
			if (GetTexture(varPair.first).size() > 0)
			{
				auto tex = GetTexture(varPair.first)[0].texture;
				shader->SetSRV(varPair.second, tex->GetShaderResourceView());
			}
		}
	}

}
