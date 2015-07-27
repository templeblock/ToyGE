#pragma once
#ifndef WINDOWSPLATFORMFACTORY_H
#define WINDOWSPLATFORMFACTORY_H

#include "ToyGE\Platform\PlatformFactory.h"

namespace ToyGE
{
	class WindowsPlatformFactory : public PlatformFactory
	{
	public:
		Ptr<Window> CreateRenderWindow(const WindowCreateParams & params) override;

		Ptr<Looper> CreateLooper() override;

		void ShowMessage(const String & msg) override;
	};
}

#endif