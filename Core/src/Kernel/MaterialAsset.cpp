#include "ToyGE\Kernel\MaterialAsset.h"
#include "ToyGE\Kernel\Core.h"
#include "ToyGE\Kernel\TextureAsset.h"
#include "ToyGE\RenderEngine\Material.h"

namespace ToyGE
{
	void MaterialAsset::Load()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to load material with empty path!");
			return;
		}

		auto file = Global::GetPlatform()->CreatePlatformFile(GetLoadPath(), FILE_OPEN_READ);
		if (!file->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open material file when reading! %s", GetLoadPath().c_str());
			return;
		}
		auto reader = std::make_shared <FileReader>(file);

		_baseColor = reader->Read<float3>();
		_roughness = reader->Read<float>();
		_metallic = reader->Read<float>();
		_emissive = reader->Read<float3>();
		_bTranslucent = reader->Read<bool>();
		_opacity = reader->Read<float>();
		_bRefraction = reader->Read<bool>();
		_refractionIndex = reader->Read<float>();
		_bDualFace = reader->Read<bool>();
		_bPOM = reader->Read<bool>();
		_pomScale = reader->Read<float>();
		_bSubsurfaceScattering = reader->Read<bool>();

		for (auto & texList : _textures)
		{
			auto numTexs = reader->Read<int32_t>();
			for (int32_t i = 0; i < numTexs; ++i)
			{
				String texRelativePath;
				reader->ReadString(texRelativePath);
				auto texCoordIndex = reader->Read<uint8_t>();

				if(texRelativePath.size() > 0)
					texList.push_back({ Asset::Find<TextureAsset>( IdenticalPath( GetPath() + "/../" + texRelativePath) ), texCoordIndex });
				else
					texList.push_back({ nullptr, texCoordIndex });
			}
		}
		_bLoaded = true;
		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Material loaded! %s", GetLoadPath().c_str());
	}

	void MaterialAsset::Init()
	{
		for (auto & texList : _textures)
		{
			for (auto & tex : texList)
			{
				if(!tex.texAsset->IsInit())
					tex.texAsset->Init();
			}
		}

		_material = std::make_shared<Material>();
		_material->SetAsset(shared_from_this()->Cast<MaterialAsset>());
		_material->SetBaseColor(_baseColor);
		_material->SetRoughness(_roughness);
		_material->SetMetallic(_metallic);
		_material->SetEmissive(_emissive);
		_material->SetTranslucent(_bTranslucent);
		_material->SetOpcacity(_opacity);
		_material->SetRefraction(_bRefraction);
		_material->SetRefractionIndex(_refractionIndex);
		_material->SetDualFace(_bDualFace);
		_material->SetPOM(_bPOM);
		_material->SetPOMScale(_pomScale);
		_material->SetSubsurfaceScattering(_bSubsurfaceScattering);

		uint32_t texType = 0;
		for (auto & texList : _textures)
		{
			for (auto & tex : texList)
			{
				_material->AddTexture(static_cast<MaterialTextureType>(texType), tex.texAsset->GetTexture(), tex.texCoordIndex);
			}
			++texType;
		}
		_material->Init();

		_material->_bDirty = false;

		_bInit = true;
	}

	void MaterialAsset::Save()
	{
		if (_path.size() == 0)
		{
			ToyGE_LOG(LT_ERROR, "Try to save material with empty path!");
			return;
		}

		// Save texture assets
		for (auto & texList : _textures)
		{
			for (auto & tex : texList)
			{
				if (tex.texAsset && tex.texAsset->IsDirty())
					tex.texAsset->Save();
			}
		}

		// Update if necessary
		if (_material && _material->_bDirty)
		{
			UpdateFromMaterial();
		}

		// Open file
		MakePathDirectorys(ParentPath(GetSavePath()));
		auto file = Global::GetPlatform()->CreatePlatformFile(GetSavePath(), FILE_OPEN_WRITE);
		if (!file->IsValid())
		{
			ToyGE_LOG(LT_WARNING, "Cannot open material file when saving! %s", GetSavePath().c_str());
			return;
		}
		auto writer = std::make_shared <FileWriter>(file);

		// Write data
		writer->Write<float3>(_baseColor);
		writer->Write<float>(_roughness);
		writer->Write<float>(_metallic);
		writer->Write<float3>(_emissive);
		writer->Write<bool>(_bTranslucent);
		writer->Write<float>(_opacity);
		writer->Write<bool>(_bRefraction);
		writer->Write<float>(_refractionIndex);
		writer->Write<bool>(_bDualFace);
		writer->Write<bool>(_bPOM);
		writer->Write<float>(_pomScale);
		writer->Write<bool>(_bSubsurfaceScattering);

		for (auto & texList : _textures)
		{
			writer->Write<int32_t>(static_cast<int32_t>(texList.size()));
			for(auto & tex : texList)
			{ 
				String texRelativePath;
				Global::GetPlatform()->GetRelativePath(
					ParentPath(GetLoadFullPath()), true,
					tex.texAsset->GetLoadFullPath(), false, 
					texRelativePath);

				writer->WriteString(texRelativePath);
				writer->Write<uint8_t>(tex.texCoordIndex);
			}
		}

		_bDirty = false;

		ToyGE_LOG(LT_INFO, "Material saved! %s", file->GetPath().c_str());
	}

	void MaterialAsset::UpdateFromMaterial()
	{
		SetBaseColor(_material->_baseColor);
		SetRoughness(_material->_roughness);
		SetMetallic(_material->_metallic);
		SetEmissive(_material->_emissive);
		SetTranslucent(_material->_bTranslucent);
		SetOpcacity(_material->_opacity);
		SetRefraction(_material->_bRefraction);
		SetRefractionIndex(_material->_refractionIndex);
		SetDualFace(_material->_bDualFace);
		SetPOM(_material->_bPOM);
		SetPOMScale(_material->_pomScale);
		SetSubsurfaceScattering(_material->_bSubsurfaceScattering);
	}
}