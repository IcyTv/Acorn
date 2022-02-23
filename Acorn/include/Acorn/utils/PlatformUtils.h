#pragma once

#include "core/Core.h"
#include "core/Log.h"

#include <string>

namespace Acorn
{
	class PlatformUtils
	{
	public:
		static std::string OpenFile(const std::vector<std::string>& filters, const std::string& title = "Open File", const std::string& initialPath = ".", bool multiselect = false);
		static std::string SaveFile(const std::vector<std::string>& filters, const std::string& title = "Save File", const std::string& initialPath = ".");
	};
}