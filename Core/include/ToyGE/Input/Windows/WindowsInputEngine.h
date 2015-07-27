#pragma once
#ifndef WINDOWSINPUTENGINE_H
#define WINDOWSINPUTENGINE_H

#include "ToyGE\Input\InputEngine.h"

namespace ToyGE
{
	class WindowsInputEngine : public InputEngine
	{
	public:
		void RefreshInputDevices() override;

		void OnRawInput(const Ptr<Window> & window, long long handle) override;
	};
}

#endif