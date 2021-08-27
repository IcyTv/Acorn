#include "acpch.h"

#include "Shader.h"

#include "renderer/Renderer.h"
#include "platform/opengl/OpenGLShader.h"

#include <fstream>
#include <filesystem>

enum ShaderState
{
	None = -1,
	Vertex = 0,
	Fragment = 1,
};

namespace Acorn
{
	std::string BasePath(const std::string& path)
	{
		size_t lastIdx = path.find_last_of("/");

		if (lastIdx == std::string::npos)
		{
			lastIdx = path.find_last_of("\\");
		}

		if (lastIdx == std::string::npos)
		{
			return "./";
		}
		else {
			return path.substr(0, lastIdx + (size_t)1);
		}
	}

	std::string FileName(const std::string& path)
	{
		size_t lastIdx = path.find_last_of("/");
		size_t lastIdxWin = path.find_last_of("\\");

		if (lastIdx == std::string::npos)
		{
			lastIdx = lastIdxWin;
		}
		else if (lastIdx < lastIdxWin)
		{
			lastIdx = lastIdxWin;
		}

		if (lastIdx == std::string::npos)
			lastIdx = 0;

		size_t dotIdx = path.find_last_of(".");

		if (dotIdx == std::string::npos)
		{
			return path.substr(lastIdx + (size_t)1);
		}
		else {
			return path.substr(lastIdx + (size_t)1, dotIdx - lastIdx - (size_t)1);
		}
	}

	void Shader::ParseShaderNoQualifier(const std::string& path, std::stringstream& stream)
	{
		AC_PROFILE_FUNCTION();

		std::ifstream file(path, std::ios::in | std::ios::binary);
		AC_CORE_ASSERT(file, "Failed to open {}", path);

		for (std::string line; getline(file, line);)
		{
			if (line.find("#include") != std::string::npos)
			{
					Include(BasePath(path), line, stream);
			}
			else
			{
				stream << line << "\n";
			}
		}
	}

	void Shader::Include(const std::string& basePath, const std::string& line, std::stringstream& stream)
	{
		AC_PROFILE_FUNCTION();

		auto startIdx = line.find_first_of("\"");
		auto endIdx = line.find_last_of("\"");
		AC_CORE_ASSERT(startIdx != std::string::npos, "No Include File found");
		AC_CORE_ASSERT(endIdx != std::string::npos, "No Include File found");
		AC_CORE_ASSERT(startIdx != endIdx, "No Include File found");

		std::string name = line.substr(startIdx + (size_t)1, endIdx - startIdx - (size_t)1);

		std::stringstream qualifiedPath;
		qualifiedPath << basePath << name;

		ParseShaderNoQualifier(qualifiedPath.str(), stream);
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetApi())
		{
		case RendererApi::Api::None:
			AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
			return nullptr;
		case RendererApi::Api::OpenGL:
			return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
		default:
			AC_CORE_ASSERT(false, "Unknown Renderer Api!");
			return nullptr;
		}
	}

	Ref<Acorn::Shader> Shader::Create(const std::string& filename)
	{
		AC_PROFILE_FUNCTION();

		std::ifstream ifs(filename, std::ios::in | std::ios::binary);

		std::stringstream shaders[2];

		AC_CORE_TRACE("Reading Shader from {0}", filename);

		ShaderState state = ShaderState::None;

		AC_CORE_ASSERT(ifs, "Could not open {}", filename);
		for (std::string line; getline(ifs, line);)
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("fragment") != std::string::npos)
				{
					state = ShaderState::Fragment;
				}
				else if (line.find("vertex") != std::string::npos)
				{
					state = ShaderState::Vertex;
				}
			}
			else if (line.find("#include") != std::string::npos)
			{
				if (state == ShaderState::None && line != "")
				{
					for (auto& ss : shaders)
					{
						Include(BasePath(filename), line, ss);
					}
				} 
				else
				{
					Include(BasePath(filename), line, shaders[state]);
				}
			}
			else
			{
				if (state == ShaderState::None && line != "")
				{
					AC_CORE_ASSERT(false, "No shader state, but tried to read in file!");
					continue;
				}
				shaders[state] << line << "\n";
			}
		}

		return Shader::Create(FileName(filename), shaders[0].str(), shaders[1].str());
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{
		AC_CORE_ASSERT(!Exists(name), "Shader with that name already exists");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		Ref<Shader> shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		Ref<Shader> shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;

	}

	void ShaderLibrary::LoadFolder(const std::string& folder)
	{
		AC_PROFILE_FUNCTION();

		std::filesystem::directory_iterator it(folder);

		AC_CORE_ASSERT(it->exists(), "Folder not found!");

		for (auto file : it)
		{
			Load(file.path().string());
		}
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		AC_CORE_ASSERT(Exists(name), "Shader does not exist");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}