#pragma once

#include "renderer/Shader.h"

#include "glm/glm.hpp"
#include <string>
#include <optional>
#include <unordered_map>

namespace Acorn
{
	class OpenGLShader: public Shader
	{
	public:
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
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

		inline virtual const std::string& GetName() const override {
			return m_Name;
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
		int GetUniformLocation(const std::string& name) const;

		std::optional<std::string> GetShaderCompilationError(uint32_t shader);
		std::optional<std::string> GetProgramLinkError(uint32_t program);
	private:
		uint32_t m_RendererId;
		std::unordered_map<std::string, int> m_UniformLocations;
		std::string m_Name;

		static uint32_t s_CurrentShader;
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);
	};

}