#include "acpch.h"

#include "utils/FileUtils.h"

#include "utils/md5.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

#ifdef AC_PLATFORM_WINDOWS
	#include <corecrt.h>
#endif // AC_PLATFORM_WINDOWS

#define ACORN_MAX_SHADER_FILE_SIZE (1024 * 1024 * 10)

namespace Acorn::Utils
{
	namespace File
	{
		// TODO think about a better way to do this
		// TODO also think about a better config file layout (maybe add to scene?)

		bool HasShaderFileChanged(const std::string& filePath)
		{
			AC_PROFILE_FUNCTION();
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
				// TODO create config file and add shader modification time
				YAML::Emitter out;
				out << YAML::BeginMap;
				out << YAML::Key << "shaders";
				out << YAML::Value << YAML::BeginMap;
				out << YAML::Key << filePath;
				out << YAML::Value << chronoModTime;
				out << YAML::EndMap; // Shaders
				out << YAML::EndMap; // Root

				std::ofstream outFile(CONFIG_FILENAME);
				outFile << out.c_str();
				outFile.close();
				return true;
			}

			YAML::Node configNode = YAML::Load(configFile);
			if (!configNode["shaders"])
			{
				// TODO Add shader modification time
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
			AC_PROFILE_FUNCTION();

			ZoneText(filePath.c_str(), filePath.length());

			FILE* file = fopen(filePath.c_str(), "rb");
			AC_CORE_ASSERT(file, "Failed to read file {}!", filePath);

			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			fseek(file, 0, SEEK_SET);

			char* buffer = new char[size + 1];
			fread(buffer, 1, size, file);
			buffer[size] = '\0';

			fclose(file);

			std::string result(buffer);
			delete[] buffer;

			return result;
		}

		void WriteFile(const std::string& filePath, const std::string& data)
		{
			AC_PROFILE_FUNCTION();
			std::ofstream file(filePath);
			if (file)
			{
				file << data;
			}
			else
			{
				AC_CORE_ASSERT(false, "Failed to open file {}!", filePath);
			}
		}

		std::string MD5HashFilePath(const std::string& filePath)
		{
			AC_PROFILE_FUNCTION();
			std::basic_ifstream<char> file(filePath);
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();
			AC_CORE_ASSERT(fileSize < ACORN_MAX_SHADER_FILE_SIZE, "Shader file is too large to hash!");
			file.seekg(0, std::ios::beg);
			// unsigned char outBuf[16]; // MD5 digest size
			std::vector<char> fileBuf(fileSize);

			if (!file.read(fileBuf.data(), fileSize))
			{
				AC_CORE_ASSERT(false, "Failed to read file!");
			}

			std::string data(fileBuf.data(), fileSize);

			std::string out = md5(data);

			// std::stringstream buffer;
			// buffer << std::hex;
			// for (int i = 0; i < 16; i++)
			// {
			// 	buffer << std::setw(2) << std::setfill('0') << (unsigned int)outBuf[i];
			// }
			// return buffer.str();
			return out;
		}

		std::string MD5HashString(const std::string& data)
		{
			AC_PROFILE_FUNCTION();
			// unsigned char outBuf[16]; // MD5 digest size
			// MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), outBuf);

			// std::stringstream buffer;
			// buffer << std::hex;
			// for (int i = 0; i < 16; i++)
			// {
			// 	buffer << std::setw(2) << std::setfill('0') << (unsigned int)outBuf[i];
			// }
			// return buffer.str();
			// return "";

			return md5(data);
		}

		std::string ResolveResPath(const std::string& filePath)
		{
			AC_PROFILE_FUNCTION();

			std::filesystem::path path(filePath);
			if (path.is_absolute())
			{
				return filePath;
			}

			if (std::filesystem::exists(filePath))
			{
				return filePath;
			}

			// TODO either cache this or enforce one res path.

			std::filesystem::path current = path;
			const size_t MAX_RECURSION = 3;
			for (size_t i = 0; i < MAX_RECURSION; i++)
			{
				current = std::filesystem::path("..") / current;
				if (std::filesystem::exists(current))
				{
					return current.string();
				}
			}

			AC_ASSERT_NOT_REACHED();
			return "";
		}

	}
}