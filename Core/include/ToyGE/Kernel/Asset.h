#pragma once
#ifndef ASSET_H
#define ASSET_H

#include "ToyGE\Kernel\PreInclude.h"
#include "ToyGE\Kernel\CorePreInclude.h"
#include "ToyGE\Kernel\StaticCastable.h"

#include <boost\noncopyable.hpp>

namespace ToyGE
{
	class TOYGE_CORE_API Asset : public StaticCastable, public boost::noncopyable
	{
	public:
		virtual ~Asset() = default;

		// Load assets
		virtual void Load() = 0;

		// Save assets
		virtual void Save() = 0;

		virtual void Init() = 0;

		static void SetAssetsBasePath(const String & basePath) { _assetsBasePath = basePath; };
		static const String & GetAssetsBasePath() { return _assetsBasePath; }

		template<class AssetType>
		static Ptr<AssetType> Find(const String & path)
		{
			auto & asset = _assetsMap[path];
			if (!asset)
			{
				asset = AssetType::New();//std::make_shared<AssetType>();
				asset->SetPath(path);
				asset->Load();
				if (!asset->IsLoaded())
					asset = nullptr;
			}

			if (asset)
				return asset->Cast<AssetType>();
			else
				return nullptr;
		}

		template<class AssetType>
		static Ptr<AssetType> FindAndInit(const String & path)
		{
			auto find = Find<AssetType>(path);
			if (find && !find->IsInit())
				find->Init();
			return find;
		}

		static const std::map<String, Ptr<Asset>> & GetAssetsMap()
		{
			return _assetsMap;
		}

		CLASS_SET(Path, String, _path);
		CLASS_GET(Path, String, _path);

		String GetLoadPath() const
		{
			if (_path.find(":") == String::npos && _assetsBasePath.size() > 0)
			{
				return _assetsBasePath + "/" + _path;
			}
			return _path;
		}

		String GetLoadFullPath() const;

		String GetSavePath() const
		{
			auto loadPath = GetLoadPath();
			if (loadPath.rfind('.') != String::npos)
				return loadPath.substr(0, loadPath.rfind('.')) + GetAssetExtension();
			else
				return loadPath + GetAssetExtension();
		}

		String GetSaveFullPath() const;

		virtual String GetAssetExtension() const { return ""; };

		bool IsLoaded() const
		{
			return _bLoaded;
		}

		bool IsInit() const
		{
			return _bInit;
		}

		CLASS_SET(Dirty, bool, _bDirty);
		bool IsDirty() const
		{
			return _bDirty;
		}

		void Register()
		{
			_assetsMap[_path] = Cast<Asset>();
		}

	protected:
		static String _assetsBasePath;
		static std::map<String, Ptr<Asset>> _assetsMap;

		String _path;
		bool _bLoaded = false;
		bool _bInit = false;
		bool _bDirty = false;
	};
}

#endif
