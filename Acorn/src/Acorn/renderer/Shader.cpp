#include "acpch.h"

#include "renderer/Shader.h"

#include "platform/opengl/OpenGLShader.h"
#include "renderer/Renderer.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include <filesystem>
#include <fstream>

enum ShaderState
{
	None = -1,
	Vertex = 0,
	Fragment = 1,
};

namespace Acorn
{

	namespace Utils::Shader
	{
		const char* GetCacheDirectory()
		{
			switch (Renderer::GetApi())
			{
				case RendererApi::Api::None:
					AC_CORE_ASSERT(false, "RendererAPI::None is not supported!");
					return nullptr;
				case RendererApi::Api::OpenGL:
					return "res/cache/shaders/opengl";
				default:
					AC_CORE_ASSERT(false, "Unknown RendererAPI!");
					return nullptr;
			}
		}

		void CreateCacheDirectory()
		{
			if (!std::filesystem::exists(GetCacheDirectory()))
				std::filesystem::create_directories(GetCacheDirectory());
		}
	}

	// Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	// {
	// 	switch (Renderer::GetApi())
	// 	{
	// 		case RendererApi::Api::None:
	// 			AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
	// 			return nullptr;
	// 		case RendererApi::Api::OpenGL:
	// 			return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
	// 		default:
	// 			AC_CORE_ASSERT(false, "Unknown Renderer Api!");
	// 			return nullptr;
	// 	}
	// }

	Ref<Acorn::Shader> Shader::Create(const std::string& filename)
	{
		AC_PROFILE_FUNCTION();

		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLShader>(filename);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}

		// std::ifstream ifs(filename, std::ios::in | std::ios::binary);

		// std::stringstream shaders[2];

		// AC_CORE_TRACE("Reading Shader from {0}", filename);

		// ShaderState state = ShaderState::None;

		// AC_CORE_ASSERT(ifs, "Could not open {}", filename);
		// for (std::string line; getline(ifs, line);)
		// {
		// 	if (line.find("#shader") != std::string::npos)
		// 	{
		// 		if (line.find("fragment") != std::string::npos)
		// 		{
		// 			state = ShaderState::Fragment;
		// 		}
		// 		else if (line.find("vertex") != std::string::npos)
		// 		{
		// 			state = ShaderState::Vertex;
		// 		}
		// 	}
		// 	else if (line.find("#include") != std::string::npos)
		// 	{
		// 		if (state == ShaderState::None && line != "")
		// 		{
		// 			for (auto& ss : shaders)
		// 			{
		// 				Include(BasePath(filename), line, ss);
		// 			}
		// 		}
		// 		else
		// 		{
		// 			Include(BasePath(filename), line, shaders[state]);
		// 		}
		// 	}
		// 	else
		// 	{
		// 		if (state == ShaderState::None && line != "")
		// 		{
		// 			AC_CORE_ASSERT(false, "No shader state, but tried to read in file!");
		// 			continue;
		// 		}
		// 		shaders[state] << line << "\n";
		// 	}
		// }

		// return Shader::Create(FileName(filename), shaders[0].str(), shaders[1].str());
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