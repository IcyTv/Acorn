#include "SourceGenerator.h"

#include <filesystem>
#include <fmt/format.h>

namespace Acorn::IDL
{
	static std::filesystem::path ResolvePath(std::filesystem::path resolver)
	{
		const auto path = std::filesystem::path("Acorn/tools/IDLParser/src/");
		return path / resolver;
	}

	static std::string toPascalCase(std::string_view str)
	{
		std::string result;
		result.reserve(str.size());

		bool first = true;
		for (auto c : str)
		{
			if (c == '_')
			{
				first = true;
				continue;
			}

			if (first)
			{
				result += toupper(c);
				first = false;
			}
			else
			{
				result += tolower(c);
			}
		}

		return result;
	}

	SourceGenerator::SourceGenerator()
	{
		m_Environment.add_callback(
			"to_pascal_case",
			[](inja::Arguments& args)
			{
				if (args.size() != 1)
					throw new std::runtime_error("to_pascal_case expects 1 argument");

				auto str = args.at(0)->get<std::string>();
				return toPascalCase(str);
			}
		);

		m_Environment.add_callback(
			"join_with_range",
			[](inja::Arguments& args)
			{
				if (args.size() != 3)
					throw new std::runtime_error("join_with_range expects 3 arguments");

				auto seperator = args.at(0)->get<std::string>();
				auto pattern   = args.at(1)->get<std::string>();
				auto size	   = args.at(2)->get<int>();

				std::stringstream ss;
				for (int i = 0; i < size; ++i)
				{
					ss << fmt::format(pattern, i);
					if (i != size - 1)
						ss << seperator;
				}

				return ss.str();
			}
		);
	}

	void SourceGenerator::Add(std::string_view name, std::string_view value)
	{
		m_Data.emplace(name, value);
	}

	void SourceGenerator::Add(const inja::json& data)
	{
		m_Data.update(data);
	}

	void SourceGenerator::Append(std::string_view tpl)
	{
		// m_Environment.render_to(m_Buffer, data, m_Data);
		m_Buffer << m_Environment.render(tpl, m_Data);
	}

	void SourceGenerator::Append(const inja::Template& tpl)
	{
		// m_Environment.render_to(m_Buffer, data, m_Data);
		m_Buffer << m_Environment.render(tpl, m_Data);
	}

	void SourceGenerator::AppendFile(const std::filesystem::path& path)
	{
		auto p = ResolvePath(path);
		if (!std::filesystem::exists(p))
			throw new std::runtime_error("File does not exist");

		m_Buffer << m_Environment.render_file(p.string(), m_Data);
	}

	std::string SourceGenerator::Generate() const
	{
		return m_Buffer.str();
	}

} // namespace Acorn::IDL