#pragma once

#include "GenericLexer.h"

#include <inja/inja.hpp>

#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Acorn::IDL
{
	class SourceGenerator
	{
		public:
			SourceGenerator() = delete;
			explicit SourceGenerator(std::string templatePath);

			void Add(std::string_view name, std::string_view value);
			void Add(const inja::json& data);

			void Append(std::string_view tpl);
			void Append(const inja::Template& tpl);
			void AppendFile(const std::filesystem::path& path);

			std::string Generate() const;

		private:
			std::stringstream m_Buffer;
			inja::Environment m_Environment;
			inja::json m_Data;

	};

} // namespace Acorn::IDL