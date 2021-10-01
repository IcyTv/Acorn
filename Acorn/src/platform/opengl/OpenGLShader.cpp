#include "acpch.h"

#include "OpenGLShader.h"

#include "Acorn/debug/Timer.h"
#include "Acorn/utils/FileUtils.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <filesystem>
#include <fstream>

#include <TracyOpenGL.hpp>

#ifdef AC_DEBUG
	#define CHECK_PROGRAM_BOUND                 \
		int id;                                 \
		glGetIntegerv(GL_CURRENT_PROGRAM, &id); \
		AC_CORE_ASSERT(id, "No program bound"); \
		AC_CORE_ASSERT(id == m_RendererId, "Wrong Shader Program bound {}", id);
#else
	#define CHECK_PROGRAM_BOUND
#endif

namespace Acorn
{
	namespace Utils::Shader
	{
		const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER:
					return "GL_FRAGMENT_SHADER";
				case GL_GEOMETRY_SHADER:
					return "GL_GEOMETRY_SHADER";
				case GL_TESS_CONTROL_SHADER:
					return "GL_TESS_CONTROL_SHADER";
				case GL_TESS_EVALUATION_SHADER:
					return "GL_TESS_EVALUATION_SHADER";
				case GL_COMPUTE_SHADER:
					return "GL_COMPUTE_SHADER";
				default:
					AC_CORE_ASSERT(false, "Unknown shader stage: {}", stage);
					return "";
			}
		}

		GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;
			AC_CORE_ASSERT(false, "Unknown shader type: {}", type);
			return 0;
		}

		shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return shaderc_vertex_shader;
				case GL_FRAGMENT_SHADER:
					return shaderc_fragment_shader;
				case GL_GEOMETRY_SHADER:
					return shaderc_geometry_shader;
				case GL_TESS_CONTROL_SHADER:
					return shaderc_tess_control_shader;
				case GL_TESS_EVALUATION_SHADER:
					return shaderc_tess_evaluation_shader;
				case GL_COMPUTE_SHADER:
					return shaderc_compute_shader;
				default:
					AC_CORE_ASSERT(false, "Unknown shader stage: {}", GLShaderStageToString(stage));
					return (shaderc_shader_kind)0;
			}
		}

		const char* GLShaderStageToCachedOpenGLFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER:
					return ".cached_opengl.frag";
				case GL_GEOMETRY_SHADER:
					return ".cached_opengl.geom";
				case GL_TESS_CONTROL_SHADER:
					return ".cached_opengl.tesc";
				case GL_TESS_EVALUATION_SHADER:
					return ".cached_opengl.tese";
				case GL_COMPUTE_SHADER:
					return ".cached_opengl.comp";
				default:
					AC_CORE_ASSERT(false, "Unknown shader stage: {}", GLShaderStageToString(stage));
					return "";
			}
		}

		const char* GLShaderStageToCachedVulkanFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:
					return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER:
					return ".cached_vulkan.frag";
				case GL_GEOMETRY_SHADER:
					return ".cached_vulkan.geom";
				default:
					AC_CORE_ASSERT(false, "Unknown shader stage: {}", GLShaderStageToString(stage));
					return "";
			}
		}
	}

	uint32_t OpenGLShader::s_CurrentShader = 0;

	// OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	// 	: m_Name(name)
	// {
	// 	AC_PROFILE_FUNCTION();

	// 	uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 	const char* source = vertexSrc.c_str();
	// 	glShaderSource(vertexShader, 1, &source, 0);

	// 	glCompileShader(vertexShader);

	// 	auto vertexError = GetShaderCompilationError(vertexShader);
	// 	AC_CORE_ASSERT(!vertexError, "Vertex Shader Compilation Error\n\t{0}", vertexError.value());

	// 	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 	source = fragmentSrc.c_str();
	// 	glShaderSource(fragmentShader, 1, &source, 0);

	// 	glCompileShader(fragmentShader);

	// 	auto fragmentError = GetShaderCompilationError(fragmentShader);
	// 	AC_CORE_ASSERT(!fragmentError, "Fragment Shader Compilation Error\n\t{0}", fragmentError.value());

	// 	m_RendererId = glCreateProgram();
	// 	glAttachShader(m_RendererId, vertexShader);
	// 	glAttachShader(m_RendererId, fragmentShader);

	// 	glLinkProgram(m_RendererId);
	// 	auto linkError = GetProgramLinkError(m_RendererId);
	// 	AC_CORE_ASSERT(!linkError, "Shader Link Error\n\t{0}", linkError.value().c_str());

	// 	glDetachShader(m_RendererId, vertexShader);
	// 	glDetachShader(m_RendererId, fragmentShader);
	// 	ParseUniforms(vertexSrc, fragmentSrc);
	// }

	OpenGLShader::OpenGLShader(const std::string& filePath)
		: m_FilePath(filePath)
	{
		AC_PROFILE_FUNCTION();

		Utils::Shader::CreateCacheDirectory();

		std::string source = Utils::File::ReadFile(filePath);
		auto shaderSources = PreProcess(source);

		{
			Timer t;

			//TODO move to main shader.h
			CompileVulkan(shaderSources);
			for (auto&& [stage, data] : m_VulkanSPIRV)
			{
				Reflect(stage, data);
			}
			CompileOpenGL();
			CreateProgram();
			ParseUniforms();
			AC_CORE_WARN("Shader compilation took {} ms", t.ElapsedMillis());
		}

		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind('.');
		auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filePath.substr(lastSlash, count);
	}

	OpenGLShader::~OpenGLShader()
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLShader::~OpenGLShader");

		glDeleteProgram(m_RendererId);
	}

	void OpenGLShader::Bind() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLShader::Bind");
		//TODO race condition?
		if (s_CurrentShader == m_RendererId)
			return;
		glUseProgram(m_RendererId);
		s_CurrentShader = m_RendererId;
	}

	void OpenGLShader::Unbind() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLShader::Unbind");
		s_CurrentShader = 0;
		glUseProgram(0);
	}

	std::optional<std::string> OpenGLShader::GetShaderCompilationError(uint32_t shader)
	{
		AC_PROFILE_FUNCTION();

		int isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE)
		{
			int length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::string infoLog;
			infoLog.reserve(length);

			glGetShaderInfoLog(shader, length, &length, &infoLog[0]);

			glDeleteShader(shader);

			return infoLog;
		}

		return std::nullopt;
	}

	std::optional<std::string> OpenGLShader::GetProgramLinkError(uint32_t program)
	{
		AC_PROFILE_FUNCTION();

		int isCompiled = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);

		if (isCompiled == GL_FALSE)
		{
			int length = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
			std::string infoLog;
			infoLog.reserve(length);

			glGetProgramInfoLog(program, length, &length, &infoLog[0]);

			glDeleteProgram(program);

			return infoLog;
		}

		return std::nullopt;
	}

	void OpenGLShader::ParseUniforms(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		AC_PROFILE_FUNCTION();

		AC_CORE_ASSERT(m_RendererId, "No Renderer Id");

		int32_t count;
		glGetProgramiv(m_RendererId, GL_ACTIVE_UNIFORMS, &count);

		std::vector<GLenum> properties;
		properties.push_back(GL_NAME_LENGTH);
		std::vector<GLint> values(properties.size());

		std::vector<GLchar> nameData;

		for (int i = 0; i < count; i++)
		{
			glGetProgramResourceiv(m_RendererId, GL_UNIFORM, i, (int)properties.size(), properties.data(), (int)values.size(), NULL, values.data());
			nameData.resize(values[0]);
			glGetProgramResourceName(m_RendererId, GL_UNIFORM, i, (int)nameData.size(), NULL, nameData.data());
			std::string name((char*)&nameData[0], nameData.size() - 1);

			int location = glGetUniformLocation(m_RendererId, name.c_str());
			m_UniformLocations.emplace(name, location);
		}

		//std::istringstream vS(vertexSrc);
		//for (std::string line; std::getline(vS, line);)
		//{

		//	if (line.find("uniform") != std::string::npos)
		//	{
		//		std::smatch matches;
		//		std::regex_search(line, )
		//		//auto lastSpace = line.find_last_of(" ");
		//		//auto semicolon = line.find_last_of(";");
		//		//std::string name = line.substr(lastSpace + 1, semicolon - lastSpace - 1);
		//		//int location = glGetUniformLocation(m_RendererId, name.c_str());
		//		//m_UniformLocations.emplace(name, location);
		//		//AC_CORE_TRACE("Uniform {}", name);
		//	}
		//}

		//std::istringstream fS(fragmentSrc);
		//for (std::string line; std::getline(fS, line);)
		//{

		//	if (line.find("uniform") != std::string::npos)
		//	{
		//		auto lastSpace = line.find_last_of(" ");
		//		auto semicolon = line.find_last_of(";");
		//		std::string name = line.substr(lastSpace + 1, semicolon - lastSpace - 1);
		//		int location = glGetUniformLocation(m_RendererId, name.c_str());
		//		m_UniformLocations.emplace(name, location);
		//		AC_CORE_TRACE("Uniform {}", name);
		//	}
		//}
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		AC_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#shader";

		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			AC_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			AC_CORE_ASSERT(Utils::Shader::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			AC_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");

			pos = source.find(typeToken, nextLinePos);

			shaderSources[Utils::Shader::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::CompileVulkan(std::unordered_map<uint32_t, std::string> shaderSources)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		// options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		std::filesystem::path cacheDirectory = Utils::Shader::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::Shader::GLShaderStageToCachedVulkanFileExtension(stage));

			std::fstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in && !Utils::File::HasShaderFileChanged(m_FilePath))
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, Utils::Shader::GLShaderStageToShaderC(stage), shaderFilePath.string().c_str(), options);

				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AC_CORE_ERROR("Shader Stage: {}", Utils::Shader::GLShaderStageToString(stage));
					AC_CORE_ERROR(result.GetErrorMessage());
					AC_CORE_ASSERT(false, "Shader compilation failed");
				}

				shaderData[stage] = std::vector<uint32_t>(result.cbegin(), result.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out)
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CompileOpenGL()
	{
		AC_PROFILE_FUNCTION();

		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		// options.SetOptimizationLevel(shaderc_optimization_level_performance);
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		std::filesystem::path cacheDirectory = Utils::Shader::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();

		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::Shader::GLShaderStageToCachedOpenGLFileExtension(stage));

			std::fstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in && !Utils::File::HasShaderFileChanged(m_FilePath))
			{
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glsl(spirv);
				m_OpenGLSourceCode[stage] = glsl.compile();

				shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_OpenGLSourceCode[stage], Utils::Shader::GLShaderStageToShaderC(stage), shaderFilePath.string().c_str(), options);

				if (result.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					AC_CORE_ERROR(result.GetErrorMessage());
					AC_CORE_ASSERT(false, "Shader compilation failed");
				}

				shaderData[stage] = std::vector<uint32_t>(result.cbegin(), result.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out)
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		AC_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		std::vector<GLuint> shaderIds;

		for (auto&& [stage, source] : m_OpenGLSPIRV)
		{
			GLuint shader = shaderIds.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source.data(), (GLsizei)(source.size() * sizeof(uint32_t)));
			glSpecializeShader(shader, "main", 0, nullptr, nullptr);
			glAttachShader(program, shader);
		}

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

			AC_CORE_ASSERT(false, "Shader Linking failed ({0}):\n {1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIds)
			{
				glDeleteShader(id);
			}
		}

		for (auto id : shaderIds)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
		m_RendererId = program;
	}

	void OpenGLShader::ParseUniforms()
	{
		// auto iter = m_UniformLocations.begin();
		// auto endIter = m_UniformLocations.end();
		// for (; iter != endIter; ++iter)
		// {
		// 	uint32_t location = glGetUniformLocation(m_RendererId, iter->first.c_str());
		// 	AC_CORE_ASSERT(location != GL_INVALID_INDEX, "Uniform {0} not found in shader {1}", iter->first, m_FilePath);
		// 	iter->second = location;
		// }

		// GLint uniformCount = 0;
		// glGetProgramInterfaceiv(m_RendererId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);

		// AC_CORE_TRACE("{0} Uniforms in shader {1}:", uniformCount, m_FilePath);

		// for (uint32_t i = 0; i < (uint32_t)uniformCount; i++)
		// {
		// 	int size;
		// 	GLenum attribs[1] = {GL_NAME_LENGTH};
		// 	glGetProgramResourceiv(m_RendererId, GL_UNIFORM, i, 1, attribs, 1, nullptr, &size);

		// 	std::vector<GLchar> name(size);
		// 	glGetProgramResourceName(m_RendererId, GL_UNIFORM, i, size, nullptr, name.data());

		// 	GLint location = glGetUniformLocation(m_RendererId, name.data());
		// 	AC_CORE_ASSERT(location != GL_INVALID_INDEX, "Uniform {0} not found in shader {1}", name.data(), m_FilePath);
		// 	AC_CORE_TRACE("Found Uniform {}", name.data());

		// 	m_UniformLocations[name.data()] = location;
		// }
	}

	void OpenGLShader::Reflect(uint32_t stage, const std::vector<uint32_t>& shaderData)
	{
		AC_PROFILE_FUNCTION();

		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		AC_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::Shader::GLShaderStageToString(stage), m_FilePath);
		AC_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		AC_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		AC_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			size_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			size_t memberCount = bufferType.member_types.size();

			// m_UniformLocations.emplace(resource.name, -2);

			AC_CORE_TRACE("  {0}", resource.name);
			AC_CORE_TRACE("    Size = {0}", bufferSize);
			AC_CORE_TRACE("    Binding = {0}", binding);
			AC_CORE_TRACE("    Members = {0}", memberCount);

			//TODO parse uniforms and add to m_UniformLocations
		}
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& mat) const
	{

		CHECK_PROGRAM_BOUND;
		int location = GetUniformLocation(name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& mat) const
	{
		CHECK_PROGRAM_BOUND;
		int location = GetUniformLocation(name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int val) const
	{

		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform1i(location, val);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float val) const
	{

		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform1f(location, val);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& val) const
	{
		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform2fv(location, 1, glm::value_ptr(val));
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& val) const
	{
		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform3fv(location, 1, glm::value_ptr(val));
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vec) const
	{
		CHECK_PROGRAM_BOUND
		int location = GetUniformLocation(name);
		glUniform4fv(location, 1, glm::value_ptr(vec));
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		auto got = m_UniformLocations.find(name);

		int location = -1;
		if (got == m_UniformLocations.end())
		{
			AC_CORE_WARN("Unused Uniform found: {0}", name);
		}
		else
		{
			location = got->second;
		}

		return location;
	}

}