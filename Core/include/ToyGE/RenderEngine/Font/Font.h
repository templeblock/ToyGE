#pragma once
#ifndef FONT_H
#define FONT_H

#include "ToyGE\Kernel\FontAsset.h"
#include "ToyGE\Kernel\StaticCastable.h"

#include <boost\noncopyable.hpp>

namespace ToyGE
{
	class TOYGE_CORE_API Font : public StaticCastable, public boost::noncopyable
	{
	public:
		Font() {};

		virtual ~Font() = default;

		virtual void Init() = 0;

		virtual Ptr<class FontRenderer> CreateRenderer() = 0;

		void SetAsset(const Ptr<FontAsset> & asset)
		{
			_asset = asset;
		}
		Ptr<FontAsset> GetAsset() const
		{
			return _asset.lock();
		}

		virtual void SaveCache() {}

		static Ptr<Font> Find(const String & fontPath)
		{
			auto fontAsset = Asset::Find<FontAsset>(fontPath);
			if (fontAsset)
			{
				if(!fontAsset->IsInit())
					fontAsset->Init();
				return fontAsset->GetFont();
			}
			return nullptr;
		}

	protected:
		bool _bInit = false;

		std::weak_ptr<FontAsset> _asset;
	};
}

#endif
