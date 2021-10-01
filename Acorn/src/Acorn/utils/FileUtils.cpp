#include "acpch.h"

#include "FileUtils.h"

#include <chrono>
#include <corecrt.h>
#include <filesystem>
#include <fstream>
#include <openssl/md5.h>
#include <yaml-cpp/yaml.h>

namespace Acorn::Utils
{
	namespace File
	{
		//TODO think about a better way to do this
		//TODO also think about a better config file layout (maybe add to scene?)

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
			AC_PROFILE_FUNCTION();

			ZoneText(filePath.c_str(), filePath.length());
			// std::ifstream file(filePath);
			// std::stringstream buffer;
			// if (file)
			// {
			// 	buffer << file.rdbuf();
			// }
			// else
			// {
			// 	AC_CORE_ASSERT(false, "Failed to open file!");
			// }
			// return buffer.str();

			FILE* file;
			errno_t err;
			err = fopen_s(&file, filePath.c_str(), "rb");

			AC_CORE_ASSERT(err == 0, "Failed to open file!");

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
				AC_CORE_ASSERT(false, "Failed to open file!");
			}
		}

		std::string MD5HashFilePath(const std::string& filePath)
		{
			AC_PROFILE_FUNCTION();
			std::basic_ifstream<unsigned char> file(filePath);
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);
			unsigned char outBuf[16]; //MD5 digest size
			std::vector<unsigned char> fileBuf(fileSize);

			if (!file.read(fileBuf.data(), fileSize))
			{
				AC_CORE_ASSERT(false, "Failed to read file!");
			}

			MD5(fileBuf.data(), fileSize, outBuf);

			std::stringstream buffer;
			buffer << std::hex;
			for (int i = 0; i < 16; i++)
			{
				buffer << std::setw(2) << std::setfill('0') << (unsigned int)outBuf[i];
			}
			return buffer.str();
		}

		std::string MD5HashString(const std::string& data)
		{
			AC_PROFILE_FUNCTION();
			unsigned char outBuf[16]; //MD5 digest size
			MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), outBuf);

			std::stringstream buffer;
			buffer << std::hex;
			for (int i = 0; i < 16; i++)
			{
				buffer << std::setw(2) << std::setfill('0') << (unsigned int)outBuf[i];
			}
			return buffer.str();
		}
	}
}