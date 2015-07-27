#pragma once
#ifndef RAWDATAMAP_H
#define RAWDATAMAP_H

#include "ToyGE\Kernel\PreIncludes.h"
#include "ToyGE\Kernel\CorePreDeclare.h"

namespace ToyGE
{
	class RawDataBase
	{

	};

	template <class T>
	class RawData : public RawDataBase
	{
	public:
		T value;

		RawData(const T & v) : value(v){}
	};

	class RawDataMap
	{
	public:

		template <class T>
		void SetValue(const String & key, const T & value)
		{
			_dataMap[key] = std::make_shared<RawData<T>>(value);
		}

		template <class T>
		void AddValue(const String & key, const T & value)
		{
			auto dataFind = _dataMap.find(key);

			if (dataFind == _dataMap.end())
				SetValue(key, value);
			else
				std::static_pointer_cast<RawData<T>>(dataFind->second)->value += value;
		}

		template <class T>
		const T & GetValue(const String & key) const
		{
			return std::static_pointer_cast<RawData<T>>(_dataMap.find(key)->second)->value;
		}

	private:
		std::map< String, Ptr<RawDataBase> > _dataMap;
	};
}

#endif