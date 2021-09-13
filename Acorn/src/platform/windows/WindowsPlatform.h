#pragma once

#include "Acorn/core/Platform.h"

namespace Acorn
{

	class WindowsPlatform : public Platform
	{
	public:
		WindowsPlatform() {}

		virtual float GetTimeImpl() const override;
		virtual const char* GetNameImpl() const override;
	};
}