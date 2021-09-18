#include "acpch.h"

#include "FileUtils.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Acorn::Utils
{
	namespace File
	{
		//TODO think about a better way to do this
		//TODO also thingk about a better config file layout (maybe add to scene?)

		bool HasShaderFileChanged(const std::string& filePath)
		{
			std::filesystem::path fsPath(filePath);

			if (!std::filesystem::exists(fsPath))
			{
				AC_CORE_ASSERT(false, "Failed to find shader file!");
				return true;
			}

			auto fileModificationTime = std::filesystem::last_write_time(fsPath);
			auto chronoModTime = std::chrono::duration_cast<std::chrono::seconds>(fileModificationTime.time_since_epoch()).count();

			std::ifstream configFile(CONFIG_FILENAME);
			if (!configFile.is_open())
			{
				//TODO create config file and add shader modification time
				YAML::Emitter out;
				out << YAML::BeginMap;
				out << YAML::Key << "shaders";
				out << YAML::Value << YAML::BeginMap;
				out << YAML::Key << filePath;
				out << YAML::Value << chronoModTime;
				out << YAML::EndMap; //Shaders
				out << YAML::EndMap; //Root

				std::ofstream outFile(CONFIG_FILENAME);
				outFile << out.c_str();
				outFile.close();
				return true;
			}

			YAML::Node configNode = YAML::Load(configFile);
			if (!configNode["shaders"])
			{
				//TODO Add shader modification time
				configNode["shaders"] = YAML::Node(YAML::NodeType::Map);
				configNode["shaders"][filePath.c_str()] = chronoModTime;

				YAML::Emitter out;
				std::ofstream outFile(CONFIG_FILENAME);
				out << configNode;
				outFile << out.c_str();
				outFile.close();

				return true;
			}

			auto modificationTime = configNode["shaders"][filePath.c_str()];
			if (!modificationTime)
			{
				configNode["shaders"][filePath.c_str()] = chronoModTime;

				YAML::Emitter out;
				std::ofstream outFile(CONFIG_FILENAME);
				out << configNode;
				outFile << out.c_str();
				outFile.close();

				return true;
			}

			auto savedTime = modificationTime.as<int64_t>();

			configNode["shaders"][filePath.c_str()] = chronoModTime;

			YAML::Emitter out;
			std::ofstream outFile(CONFIG_FILENAME);
			out << configNode;
			outFile << out.c_str();
			outFile.close();

			return chronoModTime > savedTime;
		}

		std::string ReadFile(const std::string& filePath)
		{
			std::ifstream file(filePath);
			std::stringstream buffer;
			if (file)
			{
				buffer << file.rdbuf();
			}
			else
			{
				AC_CORE_ASSERT(false, "Failed to open file!");
			}
			return buffer.str();
		}

		void WriteFile(const std::string& filePath, const std::string& data)
		{
			std::ofstream file(filePath);
			if (file)
			{
				file << data;
			}
			else
			{
				AC_CORE_ASSERT(false, "Failed to open file!");
			}
		}
	}
}