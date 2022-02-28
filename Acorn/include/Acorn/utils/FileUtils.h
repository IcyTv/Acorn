#pragma once

#include "core/Core.h"

#include <chrono>

namespace Acorn::Utils
{
	namespace File
	{
		constexpr const char* CONFIG_FILENAME = "acorn-project.yaml";
		bool HasShaderFileChanged(const std::string& filePath);

		std::string ReadFile(const std::string& filePath);
		void WriteFile(const std::string& filePath, const std::string& data);

		std::string MD5HashFilePath(const std::string& filePath);
		std::string MD5HashString(const std::string& data);

		std::string ResolveResPath(const std::string& filePath);

		enum class FileStatus
		{
			Created,
			Modified,
			Erased,
		};
	}
}