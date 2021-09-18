#pragma once

#include "core/Core.h"

namespace Acorn::Utils
{
	namespace File
	{
		constexpr const char* CONFIG_FILENAME = "acorn-project.yaml";
		bool HasShaderFileChanged(const std::string& filePath);

		std::string ReadFile(const std::string& filePath);
		void WriteFile(const std::string& filePath, const std::string& data);
	}
}