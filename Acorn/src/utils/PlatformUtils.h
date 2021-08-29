#pragma once

#include "core/Core.h"
#include "core/Log.h"

#include <string>

namespace Acorn
{
	class PlatformUtils
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};
}