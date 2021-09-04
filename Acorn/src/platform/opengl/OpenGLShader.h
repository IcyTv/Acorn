#pragma once

#include "renderer/Shader.h"

#include "glm/glm.hpp"
#include <optional>
#include <shaderc/shaderc.hpp>
#include <string>
#include <unordered_map>

namespace Acorn
{
	namespace Utils::Shader
	{
		const char* GLShaderStageToString(uint32_t stage);
		uint32_t ShaderTypeFromString(const std::string& type);
		shaderc_shader_kind GLShaderStageToShaderC(uint32_t stage);
		const char* GLShaderStageToCachedOpenGLFileExtension(uint32_t stage);
		const char* GLShaderStageToCachedVulkanFileExtension(uint32_t stage);
	}

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		// OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual void SetMat4(const std::string& name, const glm::mat4& value) override
		{
			AC_PROFILE_FUNCTION();

			UploadUniformMat4(name, value);
		}
		inline virtual void SetFloat3(const std::string& name, const glm::vec3& value) override
		{
			AC_PROFILE_FUNCTION();

			UploadUniformFloat3(name, value);
		}
		inline virtual void SetFloat4(const std::string& name, const glm::vec4& value) override
		{
			AC_PROFILE_FUNCTION();

			UploadUniformFloat4(name, value);
		}
		inline virtual void SetInt(const std::string& name, int value) override
		{
			AC_PROFILE_FUNCTION();

			UploadUniformInt(name, value);
		}

		inline virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override
		{
			UploadUniformIntArray(name, values, count);
		}
		inline void SetFloat(const std::string& name, float value) override
		{
			AC_PROFILE_FUNCTION();

			UploadUniformFloat(name, value);
		}

		inline virtual const std::string& GetName() const override
		{
			return m_Name;
		}

		inline virtual uint32_t GetId() const override
		{
			return m_RendererId;
		}

		void UploadUniformMat3(const std::string& name, const glm::mat3& mat) const;
		void UploadUniformMat4(const std::string& name, const glm::mat4& mat) const;

		void UploadUniformInt(const std::string& name, int val) const;
		void UploadUniformFloat(const std::string& name, float val) const;
		void UploadUniformFloat2(const std::string& name, const glm::vec2& val) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& val) const;
		void UploadUniformFloat4(const std::string& name, const glm::vec4& mat) const;
		void ParseUniforms(const std::string& vertexSrc, const std::string& fragmentSrc);

	private:
		std::unordered_map<uint32_t, std::string> PreProcess(const std::string& source);
		void CompileVulkan(std::unordered_map<uint32_t, std::string> shaderSources);
		void CompileOpenGL();
		void CreateProgram();
		void ParseUniforms();

		void Reflect(uint32_t stage, const std::vector<uint32_t>& shaderData);

		int GetUniformLocation(const std::string& name) const;

		std::optional<std::string> GetShaderCompilationError(uint32_t shader);
		std::optional<std::string> GetProgramLinkError(uint32_t program);

	private:
		uint32_t m_RendererId;
		std::unordered_map<std::string, int> m_UniformLocations;
		std::string m_Name;
		std::string m_FilePath;

		static uint32_t s_CurrentShader;
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<uint32_t, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<uint32_t, std::string> m_OpenGLSourceCode;
	};

}