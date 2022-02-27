#pragma once

#include "GenericLexer.h"

#include <inja/inja.hpp>

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Acorn::IDL
{
	class SourceGenerator
	{
public:
		SourceGenerator();

		void Add(std::string_view name, std::string_view value);
		void Add(const inja::json& data);

		void Append(std::string_view tpl);
		void Append(const inja::Template& tpl);

		std::string Generate() const;

private:
		std::stringstream m_Buffer;
		inja::Environment m_Environment;
		inja::json m_Data;
	};

} // namespace Acorn::IDL