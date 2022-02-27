#include "SourceGenerator.h"

namespace Acorn::IDL
{
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

	std::string SourceGenerator::Generate() const
	{
		return m_Buffer.str();
	}

} // namespace Acorn::IDL