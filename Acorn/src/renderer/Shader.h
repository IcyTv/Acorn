#pragma once

#include "core/Core.h"

namespace Acorn
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;

		virtual void SetFloat(const std::string& name, float value) = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& filename); //* maybe move to platform specific, but for now I don't see the reason
	
	private:
		static void ParseShaderNoQualifier(const std::string& path, std::stringstream& stream);
		static void Include(const std::string& basePath, const std::string& line, std::stringstream& stream);
	};

	class ShaderLibrary
	{
	public:
		ShaderLibrary() { }

		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);
		void LoadFolder(const std::string& folder);

		Ref<Shader> Get(const std::string& name);

		Ref<Shader> operator[](const std::string& name) { return Get(name); }
		
		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}