#pragma once
#ifndef WINDOWSDLLLOADER_H
#define WINDOWSDLLLOADER_H

#include "ToyGE\Platform\DllLoader.h"

namespace ToyGE
{
	class WindowsDllLoader : public DllLoader
	{
	public:
		Ptr<DllObj> LoadDll(const String & name) override;
	};
}

#endif