#pragma once
#ifndef WINDOWSDLLOBJ_H
#define WINDOWSDLLOBJ_H

#include "ToyGE\Platform\DllObj.h"

namespace ToyGE
{
	class WindowsDllObj : public DllObj
	{
	public:
		static Ptr<DllObj> Create(HDLL dll);

		~WindowsDllObj();

		void * GetProcAddress(const String & name) override;

		operator bool()
		{
			return  nullptr != _hDll;
		}

	protected:
		WindowsDllObj(HDLL dll);


	private:
		HDLL _hDll;
	};
}

#endif