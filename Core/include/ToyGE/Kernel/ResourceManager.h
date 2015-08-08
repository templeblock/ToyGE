#pragma once
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"
#include "ToyGE\Kernel\Logger.h"
#include "ToyGE\Kernel\Assert.h"
#include "ToyGE\Kernel\File.h"
#include "ToyGE\Kernel\StaticCastable.h"
#include "ToyGE\RenderEngine\RenderCommonDefines.h"
#include "ToyGE\Kernel\Util.h"

namespace ToyGE
{
	enum ResourceManagerType : uint32_t
	{
		RESOURCE_EFFECT = 0UL,
		RESOURCE_TEXTURE = 1UL,
		RESOURCE_MATERIAL = 2UL,
		RESOURCE_MESH = 3UL,
		RESOURCE_MODEL = 4UL,
		RESOURCE_FONT = 5UL,

		RESOURCE_RESERVE = RESOURCE_FONT + 1
	};

	template <typename ResType, typename KeyType0, typename... KeyType>
	class ResourceMapLoop
	{
	public:
		using MapType = std::map < KeyType0, typename ResourceMapLoop<ResType, KeyType...>::MapType >;
	};

	template <typename ResType, typename KeyType0>
	class ResourceMapLoop<ResType, KeyType0>
	{
	public:
		using MapType = std::map < KeyType0, ResType >;
	};

	template <typename ResType, typename... KeyType>
	class ResourceMap
	{
	public:
		using MapType = typename ResourceMapLoop<ResType, KeyType...>::MapType;
	};

	class ResourceManagerBase : public StaticCastable
	{
	public:
		virtual ~ResourceManagerBase() = default;

		virtual void Clear(){};
	};

	template <typename MapType, typename ResType, typename... KeyType>
	class ResourceManager : public ResourceManagerBase
	{
	public:
		virtual ~ResourceManager() = default;

		const ResType & AcquireResource(const KeyType &... key)
		{
			auto & res = Find(key...);
			if (!res)
			{
				res = DoLoadResource(key...);
			}
			return res;
		}

		void SetResource(const ResType & res, const KeyType &... key)
		{
			Find(key...) = res;
		}

		virtual bool Exists(const KeyType &... key) const = 0;

	protected:
		MapType _resMap;

		virtual ResType DoLoadResource(const KeyType &... key) = 0;
		virtual ResType & Find(const KeyType &... key) = 0;
	};

	template <typename ResType, typename... KeyType>
	class ResourceManagerDefaultMap : public ResourceManager<typename ResourceMap<ResType, KeyType...>::MapType, ResType, KeyType...>
	{
	public:
		virtual ~ResourceManagerDefaultMap() = default;

		virtual bool Exists(const KeyType &... key) const override
		{
			return _Exists(_resMap, key...);
		}

		virtual void Clear() override
		{
			_resMap.clear();
		}

	protected:
		virtual ResType DoLoadResource(const KeyType &... key) = 0;

		ResType & Find(const KeyType &... key)
		{
			return _Find(_resMap, key...);
		}

	private:
		template <typename MapType, typename FindKey0, typename... FindKey>
		ResType & _Find(
			//typename ResourceMap<ResType, FindKey0, FindKey...>::MapType & findMap,
			MapType & findMap,
			const FindKey0 & key0,
			const FindKey &... key)
		{
			auto & nextMap = findMap[key0];
			return Find<FindKey...>(nextMap, key...);
		}

		template <typename FindKey0>
		ResType & _Find(
			std::map<FindKey0, ResType> & findMap,
			const FindKey0 & key0)
		{
			return findMap[key0];
		}

		template <typename MapType, typename FindKey0, typename... FindKey>
		bool _Exists(
			//const typename ResourceMap<ResType, FindKey0, FindKey...>::MapType & findMap,
			const MapType & findMap,
			const FindKey0 & key0,
			const FindKey &... key) const
		{
			auto find = findMap.find(key0);
			if (find != findMap.end())
				return _Exists(find->second, key...);
			else
				return false;
		}

		template <typename FindKey0>
		bool _Exists(
			const std::map<FindKey0, ResType> & findMap,
			const FindKey0 & key0) const
		{
			return findMap.find(key0) != findMap.end();
		}
	};


	class PathBased
	{
	public:
		PathBased(){}

		virtual ~PathBased() = default;

		PathBased(const WString & basePath) : _basePath(basePath)
		{
			IdenticalPath(_basePath);
			if (_basePath.back() != '\\')
				_basePath.push_back('\\');
		}

		void SetBasePath(const WString & basePath)
		{
			_basePath = basePath;
		}
		const WString & BasePath() const
		{
			return _basePath;
		}

	protected:
		WString _basePath;
	};


	class RenderEffect;

	class EffectManager
		: public PathBased, public ResourceManagerDefaultMap<Ptr<RenderEffect>, WString>
		//public ResourceManager<EffectManager_MapType, Ptr<RenderEffect>, String, std::vector<MacroDesc>>
	{
	public:
		//using ResourceManager<EffectManager_MapType, Ptr<RenderEffect>, String, std::vector<MacroDesc>>::AcquireResource;

		/*const Ptr<RenderEffect> & AcquireResource(const String & key)
		{
			return AcquireResource(key, std::vector<MacroDesc>());
		}

		bool Exists(const String & name, const std::vector<MacroDesc> & macros) const override;*/

		void Clear() override;

	protected:
		//Ptr<RenderEffect> & Find(const String & key, const std::vector<MacroDesc> & macros) override;

		Ptr<RenderEffect> DoLoadResource(const WString & key) override;
	};


	class Texture;

	class TextureManager : public PathBased, public ResourceManagerDefaultMap<Ptr<Texture>, WString>
	{
	protected:
		Ptr<Texture> DoLoadResource(const WString & key) override;
	};


	class Material;

	class MaterialManager : public ResourceManagerDefaultMap<Ptr<Material>, WString>
	{
	protected:
		Ptr<Material> DoLoadResource(const WString & key) override{ return Ptr<Material>(); }
	};


	class Mesh;

	class MeshManager : public ResourceManagerDefaultMap<Ptr<Mesh>, WString>
	{
	protected:
		Ptr<Mesh> DoLoadResource(const WString & key) override{ return Ptr<Mesh>(); }
	};


	class Model;

	class ModelManager : public PathBased, public ResourceManagerDefaultMap<Ptr<Model>, WString>
	{
	protected:
		Ptr<Model> DoLoadResource(const WString & key) override;
	};


	class Font;

	class FontManager : public PathBased, public ResourceManagerDefaultMap<Ptr<Font>, WString>
	{
	protected:
		Ptr<Font> DoLoadResource(const WString & key) override;
	};
}

#endif