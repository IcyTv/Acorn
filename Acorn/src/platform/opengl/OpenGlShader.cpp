#include "acpch.h"

#include "OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <regex>

#ifdef AC_DEBUG
#define CHECK_PROGRAM_BOUND int id;\
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);\
	AC_CORE_ASSERT(id, "No program bound");\
	AC_CORE_ASSERT(id == m_RendererId, "Wrong Shader Program bound {}", id);
#else
#define CHECK_PROGRAM_BOUND
#endif

namespace Acorn
{
	uint32_t OpenGLShader::s_CurrentShader = 0;

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		AC_PROFILE_FUNCTION();

		uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const char* source = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		glCompileShader(vertexShader);

		auto vertexError = GetShaderCompilationError(vertexShader);
		AC_CORE_ASSERT(!vertexError, "Vertex Shader Compilation Error\n\t{0}", vertexError.value());

		int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		source = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		glCompileShader(fragmentShader);

		auto fragmentError = GetShaderCompilationError(fragmentShader);
		AC_CORE_ASSERT(!fragmentError, "Fragment Shader Compilation Error\n\t{0}", fragmentError.value());

		m_RendererId = glCreateProgram();
		glAttachShader(m_RendererId, vertexShader);
		glAttachShader(m_RendererId, fragmentShader);

		glLinkProgram(m_RendererId);
		auto linkError = GetProgramLinkError(m_RendererId);
		AC_CORE_ASSERT(!linkError, "Shader Link Error\n\t{0}", linkError.value());

		glDetachShader(m_RendererId, vertexShader);
		glDetachShader(m_RendererId, fragmentShader);
		ParseUniforms(vertexSrc, fragmentSrc);
	}

	OpenGLShader::~OpenGLShader()
	{
		AC_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererId);
	}

	void OpenGLShader::Bind() const
	{
		AC_PROFILE_FUNCTION();

		//TODO race condition?
		if (s_CurrentShader == m_RendererId) return;
		glUseProgram(m_RendererId);
		s_CurrentShader = m_RendererId;
	}

	void OpenGLShader::Unbind() const
	{
		AC_PROFILE_FUNCTION();

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
		else {
			location = got->second;
		}


		return location;
	}

}