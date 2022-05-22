#include "SourceGenerator.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <filesystem>
#include <optional>

namespace Acorn::IDL
{
	enum class KnownV8Types
	{
		Boolean,
		Number,
		String,
		Object,
		Array,
		Function
	};

	static std::string CppToV8Callback(inja::Arguments& args)
	{
		const auto& arg					 = args.at(0)->get<std::string>();
		const auto& name				 = args.at(1)->get<std::string>();
		std::optional<KnownV8Types> type = magic_enum::enum_cast<KnownV8Types>(arg);
		if (type)
		{
			switch (*type)
			{
			case KnownV8Types::Boolean: return "v8::Boolean::New(isolate, name)";
			case KnownV8Types::Number: return "v8::Number::New(isolate, name)";
			case KnownV8Types::String: return "v8::String::NewFromUtf8(isolate, name)";
			case KnownV8Types::Object: return "v8::Object::New(isolate)";
			case KnownV8Types::Array: return "v8::Array::New(isolate)";
			case KnownV8Types::Function: return "v8::Function::New(isolate, name)";
			}
		}
		else
		{
			throw std::runtime_error(fmt::format("Unknown V8 type: {}", arg));
		}
	}

	static std::string V8ToCppCallback(inja::Arguments& args)
	{
		static const char* PATTERN = "v8::Local<v8::{0}>::Cast({1}).Value()";

		const auto& arg					 = args.at(0)->get<std::string>();
		const auto& name				 = args.at(1)->get<std::string>();
		std::optional<KnownV8Types> type = magic_enum::enum_cast<KnownV8Types>(arg);
		if (type)
		{
			switch (*type)
			{
			case KnownV8Types::Boolean: return fmt::format(PATTERN, "Boolean", name);
			case KnownV8Types::Number: return fmt::format(PATTERN, "Number", name);
			default:
				// TODO
				return "";
			}
		}
	}

	std::string ToPascalCase(std::string_view str)
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

			// Ignore already uppercased characters
			if (isupper(c))
			{
				result += c;
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

	SourceGenerator::SourceGenerator(std::string templatePath) : m_Environment(templatePath)
	{
		std::cerr << "Template path: " << templatePath << std::endl;

		m_Environment.add_callback("to_pascal_case",
			[](inja::Arguments& args)
			{
				if (args.size() != 1)
					throw new std::runtime_error("to_pascal_case expects 1 argument");

				auto str = args.at(0)->get<std::string>();
				return ToPascalCase(str);
			});

		m_Environment.add_callback("join_with_range",
			[](inja::Arguments& args)
			{
				if (args.size() != 3)
					throw new std::runtime_error("join_with_range expects 3 arguments");

				auto seperator = args.at(0)->get<std::string>();
				auto pattern   = args.at(1)->get<std::string>();
				auto size	   = args.at(2)->get<int>();

				std::stringstream ss;
				for (int i = 1; i < size + 1; ++i)
				{
					ss << fmt::format(pattern, i);
					if (i != size)
						ss << seperator;
				}

				return ss.str();
			});

		m_Environment.add_void_callback("dbgln",
			[](const inja::Arguments& args) -> void
			{
				for (const auto& arg : args)
				{
					std::cout << arg->get<std::string>() << std::endl;
				}
			});

		m_Environment.add_callback("cpp_to_v8", CppToV8Callback);
		m_Environment.add_callback("v8_to_cpp", V8ToCppCallback);

		// m_Environment.set_trim_blocks(true);
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
		m_Buffer << m_Environment.render_file(path.string(), m_Data);
	}

	std::string SourceGenerator::Generate() const
	{
		return m_Buffer.str();
	}

} // namespace Acorn::IDL