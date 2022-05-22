#pragma once

#include "Acorn/core/Platform.h"

namespace Acorn
{

	class GLFWPlatform : public Platform
	{
	public:
		GLFWPlatform() {}

		virtual float GetTimeImpl() const override;
		virtual const char* GetNameImpl() const override;
		virtual void* GetCurrentContextImpl() override;
	};

}