#include "ToyGE\RenderEngine\Material.h"
#include "ToyGE\Kernel\Global.h"
#include "ToyGE\RenderEngine\RenderEngine.h"
#include "ToyGE\RenderEngine\RenderFactory.h"
#include "ToyGE\Kernel\Image.h"
#include "ToyGE\RenderEngine\RenderUtil.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\ResourceManager.h"
#include "ToyGE\RenderEngine\RenderEffect.h"

namespace ToyGE
{
	Ptr<Material> Material::LoadBin(const Ptr<Reader> & reader, const WString & basePath)
	{
		auto mat = std::make_shared<Material>();

		reader->ReadString(mat->_name);
		int32_t texMapSize = reader->Read<int32_t>();
		for (int32_t i = 0; i < texMapSize; ++i)
		{
			MaterialTextureType texType = static_cast<MaterialTextureType>( reader->Read<uint32_t>() );
			int32_t numTex = reader->Read<int32_t>();
			for (int32_t j = 0; j < numTex; ++j)
			{
				MaterialTexture tex;
				tex.filePath = basePath;
				reader->ReadString(tex.filePath);
				tex.texCoordIndex = reader->Read<int8_t>();
				mat->_textures[texType].push_back(std::move(tex));
			}
		}

		mat->_baseColor = reader->Read<float3>();
		mat->_roughness = reader->Read<float>();
		mat->_metallic = reader->Read<float>();
		mat->_bTranslucent = reader->Read<bool>();
		mat->_opacity = reader->Read<float>();
		mat->_bRefraction = reader->Read<bool>();
		mat->_refractionIndex = reader->Read<float>();

		return mat;
	}

	Material::Material()
		:_baseColor(1.0f, 1.0f, 1.0f),
		_roughness(1.0f),
		_metallic(0.0f),
		_bTranslucent(false),
		_opacity(1.0f),
		_bRefraction(false),
		_refractionIndex(1.0f),
		_bDualFace(false),
		_bPOM(false),
		_pomScale(0.05f),
		_bSubSurfaceScattering(false)
	{

	}
	
	void Material::SaveBin(const Ptr<Writer> & writer, const WString & skipPrefixPath)
	{
		writer->WriteString(_name);
		writer->Write<int32_t>(static_cast<int32_t>(_textures.size()));
		for (auto & texList : _textures)
		{
			auto texType = texList.first;
			writer->Write<uint32_t>(texType);
			writer->Write<int32_t>(static_cast<int32_t>(texList.second.size()));
			for (auto & tex : texList.second)
			{
				writer->WriteString(tex.filePath.substr(skipPrefixPath.size()));
				writer->Write<int8_t>(tex.texCoordIndex);
			}
		}

		writer->Write<float3>(_baseColor);
		writer->Write<float>(_roughness);
		writer->Write<float>(_metallic);
		writer->Write<bool>(_bTranslucent);
		writer->Write<float>(_opacity);
		writer->Write<bool>(_bRefraction);
		writer->Write<float>(_refractionIndex);
	}

	const Ptr<RenderMaterial> & Material::InitRenderData()
	{
		_renderData = std::make_shared<RenderMaterial>(shared_from_this());
		return _renderData;
	}

	uint32_t Material::GetTypeFlags() const
	{
		uint32_t flags = 0;

		for (uint32_t i = 0; i <= MaterialTextureTypeNum::value; ++i)
		{
			flags = flags | (static_cast<uint32_t>(NumTextures(static_cast<MaterialTextureType>(i)) > 0) << i);
		}
		return flags;
	}

	void Material::BindMacros(const Ptr<RenderEffect> & effect)
	{
		auto renderMat = AcquireRender();

		static const std::vector<MaterialTextureType> matTexTypeList =
		{
			MATERIAL_TEXTURE_OPACITYMASK,
			MATERIAL_TEXTURE_BASECOLOR,
			MATERIAL_TEXTURE_ROUGHNESS,
			MATERIAL_TEXTURE_BUMP
		};
		static const std::vector<String> matTexMacroList =
		{
			"MAT_OPACITYMASK_TEX",
			"MAT_BASECOLOR_TEX",
			"MAT_ROUGHNESS_TEX",
			/*"MAT_NORMAL_TEX",
			"MAT_HEIGHT_TEX"*/
			"MAT_BUMP_TEX"
		};

		for (size_t i = 0; i < matTexTypeList.size(); ++i)
		{
			if (renderMat->NumTextures(matTexTypeList[i]) > 0)
				effect->AddExtraMacro(matTexMacroList[i], "");
			else
				effect->RemoveExtraMacro(matTexMacroList[i]);
		}
	}

	void Material::BindParams(const Ptr<RenderEffect> & effect)
	{
		//std::vector<MacroDesc> macros = effect->GetExtraMacros();

		auto renderMat = AcquireRender();

		static const std::vector<MaterialTextureType> matTexTypeList = 
		{
			MATERIAL_TEXTURE_OPACITYMASK,
			MATERIAL_TEXTURE_BASECOLOR,
			MATERIAL_TEXTURE_ROUGHNESS,
			MATERIAL_TEXTURE_BUMP
		};
		static const std::vector<String> matTexVarNameList =
		{
			"opacityMaskTex",
			"baseColorTex",
			"roughnessTex",
			"bumpTex"
		};

		for (size_t i = 0; i < matTexTypeList.size(); ++i)
		{
			if (renderMat->NumTextures(matTexTypeList[i]) > 0)
				effect->VariableByName(matTexVarNameList[i])->AsShaderResource()
					->SetValue(renderMat->GetTexture(matTexTypeList[i], 0)->CreateTextureView(0, 0));
		}

		effect->VariableByName("baseColor")->AsScalar()->SetValue(&_baseColor);
		effect->VariableByName("roughness")->AsScalar()->SetValue(&_roughness);
		effect->VariableByName("metallic")->AsScalar()->SetValue(&_metallic);
		effect->VariableByName("pomScale")->AsScalar()->SetValue(&_pomScale);
		effect->VariableByName("opacity")->AsScalar()->SetValue(&_opacity);
		effect->VariableByName("refractionIndex")->AsScalar()->SetValue(&_refractionIndex);
	}


	RenderMaterial::RenderMaterial(const Ptr<Material> & material)
	{
		auto textureManager = Global::GetResourceManager(RESOURCE_TEXTURE)->As<TextureManager>();
		for (auto itr = material->_textures.begin(); itr != material->_textures.end(); ++itr)
		{
			for (auto & tex : itr->second)
			{
				auto renderTex = textureManager->AcquireResource(tex.filePath);

				if (renderTex->Desc().mipLevels == 1)
				{
					renderTex = renderTex->CreateMips();
					textureManager->SetResource(renderTex, tex.filePath);
				}

				if (renderTex)
				{
					_textures[itr->first].push_back(renderTex);
				}
			}
		}

		/*if (material->NumTextures(MATERIAL_TEXTURE_HEIGHT) > 0 && material->NumTextures(MATERIAL_TEXTURE_NORMAL) == 0)
		{
			for (auto & tex : _textures[MATERIAL_TEXTURE_HEIGHT])
			{
				auto normal = HeightToNormal(tex);
				_textures[MATERIAL_TEXTURE_NORMAL].push_back(normal);
			}
		}*/

		if (material->NumTextures(MATERIAL_TEXTURE_SPECULAR) > 0 && material->NumTextures(MATERIAL_TEXTURE_ROUGHNESS) == 0)
		{
			for (auto & tex : _textures[MATERIAL_TEXTURE_SPECULAR])
			{
				auto roughnessTex = SpecularToRoughness(tex);
				_textures[MATERIAL_TEXTURE_ROUGHNESS].push_back(roughnessTex);
			}
		}
	}

}
