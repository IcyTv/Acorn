/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2021, Luke Wilde <lukew@serenityos.org>
 * Copyright (c) 2022, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "IDLParser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <streambuf>
#include <unordered_set>
#include <utility>

[[noreturn]] static void ReportParsingError(std::string_view message, std::string_view filename, std::string_view input, size_t offset)
{
	size_t lineno = 1;
	size_t colno = 1;
	size_t start_line = 0;
	size_t line_length = 0;

	for (size_t index = 0; index < input.length(); index++)
	{
		if (offset == index)
			colno = index - start_line + 1;

		if (input[index] == '\n')
		{
			if (index >= offset)
				break;

			start_line = index + 1;
			lineno++;
			line_length = 0;
		}
		else
		{
			line_length++;
		}
	}

	// TODO we could operate on the cerr stream directly, but this allows us to change the loggig system later
	// if necessary
	std::stringstream ss;
	ss << input.substr(start_line, line_length) << '\n';
	for (size_t i = 0; i < colno; i++)
		ss << ' ';
	ss << "\033[1;31m^\n";
	ss << filename << ":" << lineno << ": "
	   << "error: " << message << "\033[0m";

	std::cerr << ss.str();

	throw std::runtime_error("Parsing error");

	std::exit(1);
}

constexpr std::string ToTitleCase(std::string_view str)
{
	std::string result;
	result.reserve(str.length());
	bool nextUpper = true;

	for (char ch : str)
	{
		if (nextUpper)
			result.push_back(std::toupper(ch));
		else
			result.push_back(ch);
		nextUpper = ch == ' ';
	}

	return result;
}

static std::string ConvertEnumValueToCppEnumMember(std::string_view value, std::unordered_set<std::string>& namesAlreadySeen)
{
	std::stringstream ss;
	Acorn::IDL::GenericLexer lexer{value};

	while (!lexer.is_eof())
	{
		lexer.ignore_while([](auto c)
						   { return std::isspace(c) || c == '_' || c == '-'; });
		auto word = lexer.consume_while([](auto c)
										{ return std::isalnum(c); });

		if (!word.empty())
		{
			ss << ToTitleCase(word);
		}
		else
		{
			auto nonAlphaNum = lexer.consume_while([](auto c)
												   { return !std::isalnum(c); });
			if (!nonAlphaNum.empty())
				ss << "_";
		}
	}

	auto res = ss.str();
	if (res.empty())
		res = "Empty";

	while (namesAlreadySeen.count(res))
		res += "_";

	namesAlreadySeen.insert(res);
	return res;
}

namespace Acorn::IDL
{
	std::unordered_set<std::string> Parser::s_AllImportedPaths{};

	void Parser::AssertSpecific(char ch)
	{
		if (!m_Lexer.consume_specific(ch))
			ReportParsingError("Expected '" + std::string(1, ch) + "'", m_Filename, m_Input, m_Lexer.tell()); // TODO comnvert to fmt
	}

	void Parser::ConsumeWhitespace()
	{
		bool consumed = true;
		while (consumed)
		{
			consumed = m_Lexer.consume_while([](char c)
											 { return std::isspace(c); })
						   .length() > 0;

			if (m_Lexer.consume_specific("//"))
			{
				m_Lexer.consume_until("\n");
				m_Lexer.ignore();
				consumed = true;
			}
		}
	}

	void Parser::AssertString(std::string_view expected)
	{
		if (!m_Lexer.consume_specific(expected))
			ReportParsingError("Expected '" + std::string(expected) + "'", m_Filename, m_Input, m_Lexer.tell()); // TODO comnvert to fmt
	}

	std::unordered_map<std::string, std::string> Parser::ParseExtendedAttributes()
	{
		std::unordered_map<std::string, std::string> extendedAttributes;
		for (;;)
		{
			ConsumeWhitespace();
			if (m_Lexer.consume_specific(']'))
				break;

			auto name = m_Lexer.consume_until([](auto ch)
											  { return ch == ']' || ch == '=' || ch == ','; });
			if (m_Lexer.consume_specific('='))
			{
				auto value = m_Lexer.consume_until([](auto ch)
												   { return ch == ']' || ch == ','; });
				extendedAttributes.emplace(name, value);
			}
			else
			{
				extendedAttributes.emplace(name, "");
			}
			m_Lexer.consume_specific(',');
		}
		ConsumeWhitespace();

		return extendedAttributes;
	}

	std::optional<std::shared_ptr<Interface>> Parser::ResolveImport(auto path)
	{
		auto includePath = std::filesystem::path(m_Filename).parent_path() / path;
		if (!std::filesystem::exists(includePath))
		{
			ReportParsingError("Could not find import file '" + includePath.string() + "'", m_Filename, m_Input, m_Lexer.tell());
		}

		auto real_path = std::filesystem::canonical(includePath);
		if (s_AllImportedPaths.count(real_path.string()))
		{
			return {};
		}

		s_AllImportedPaths.insert(real_path.string());

		std::ifstream file(real_path);
		if (!file.is_open())
		{
			ReportParsingError("Could not open import file '" + includePath.string() + "'", m_Filename, m_Input, m_Lexer.tell());
		}
		std::string data;
		file.seekg(0, std::ios::end);
		data.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		return Parser(real_path.string(), data, m_ImportBasePath).Parse();
	}

	std::shared_ptr<Type> Parser::ParseType()
	{
		if (m_Lexer.consume_specific('('))
		{
			std::vector<std::shared_ptr<Type>> unionMemberTypes;
			unionMemberTypes.push_back(ParseType());
			ConsumeWhitespace();
			AssertString("or");
			ConsumeWhitespace();
			unionMemberTypes.push_back(ParseType());
			ConsumeWhitespace();

			while (m_Lexer.consume_specific("or"))
			{
				ConsumeWhitespace();
				unionMemberTypes.push_back(ParseType());
				ConsumeWhitespace();
			}

			AssertSpecific(')');

			bool isNullable = m_Lexer.consume_specific('?');

			return std::make_shared<UnionType>("", isNullable, std::move(unionMemberTypes));
		}

		bool unsigned_ = m_Lexer.consume_specific("unsigned");
		if (unsigned_)
			ConsumeWhitespace();

		auto name = m_Lexer.consume_until([](auto c)
										  { return !std::isalnum(c) && c != '_'; });

		std::vector<std::shared_ptr<Type>> parameters;
		bool isParameterizedType = false;

		if (m_Lexer.consume_specific('<'))
		{
			// Can look like <T,    U,   W>
			// This means we expect a "," immediatly after a type and a > immediatly after the last type
			// This is the way SerenityOS implements it, and the spec doesn't say anything about it.
			// It definitely makes the code look nicer, but maybe we should allow whitespace between...
			isParameterizedType = true;
			parameters.push_back(ParseType());
			while (m_Lexer.consume_specific(','))
			{
				ConsumeWhitespace();
				parameters.push_back(ParseType());
			}
			m_Lexer.consume_specific('>');
		}

		bool nullable = m_Lexer.consume_specific('?');
		std::stringstream builder;

		if (unsigned_)
			builder << "unsigned ";

		builder << name;

		if (isParameterizedType)
			return std::make_shared<ParameterizedType>(builder.str(), nullable, std::move(parameters));

		return std::make_shared<Type>(builder.str(), nullable);
	}

	void Parser::ParseAttribute(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface)
	{
		bool readonly = m_Lexer.consume_specific("readonly");
		if (readonly)
			ConsumeWhitespace();

		if (m_Lexer.consume_specific("attribute"))
			ConsumeWhitespace();

		auto type = ParseType();
		ConsumeWhitespace();
		std::string_view name = m_Lexer.consume_until([](auto c)
													  { return std::isspace(c) || c == ';'; });
		ConsumeWhitespace();

		AssertSpecific(';');

		auto nameStr = std::string(name);
		auto getterCallbackName = "Get" + ToTitleCase(name);
		auto setterCallbackName = "Set" + ToTitleCase(name);

		Attribute attr{
			readonly,
			std::move(type),
			std::move(nameStr),
			std::move(extendedAttributes),
			std::move(getterCallbackName),
			std::move(setterCallbackName)};

		interface.Attributes.push_back(std::move(attr));
	}

	void Parser::ParseConstant(Interface& interface)
	{
		m_Lexer.consume_specific("const");
		ConsumeWhitespace();

		auto type = ParseType();
		ConsumeWhitespace();
		auto name = m_Lexer.consume_until([](auto c)
										  { return std::isspace(c) || c == '='; });
		ConsumeWhitespace();
		m_Lexer.consume_specific('='); // TODO shouldn't this be AssertSpecific?
		ConsumeWhitespace();
		auto value = m_Lexer.consume_while([](auto c)
										   { return !std::isspace(c) && c != ';'; });
		ConsumeWhitespace();
		AssertSpecific(';');

		Constant constant{
			std::move(type),
			std::move(std::string(name)),
			std::move(std::string(value))};

		interface.Constants.push_back(std::move(constant));
	}

	std::vector<Parameter> Parser::ParseParameters()
	{
		ConsumeWhitespace();
		std::vector<Parameter> parameters;

		for (;;)
		{
			if (m_Lexer.next_is(')'))
				break;

			std::unordered_map<std::string, std::string> extendedAttributes;
			if (m_Lexer.consume_specific('['))
				extendedAttributes = ParseExtendedAttributes();
			bool optional = m_Lexer.consume_specific("optional");
			if (optional)
				ConsumeWhitespace();

			auto type = ParseType();
			bool variadic = m_Lexer.consume_specific("...");
			ConsumeWhitespace();
			auto name = m_Lexer.consume_until([](auto c)
											  { return std::isspace(c) || c == ',' || c == ')' || c == '='; });
			Parameter parameter{
				std::move(type),
				std::move(std::string(name)),
				optional,
				{},
				std::move(extendedAttributes),
				variadic};
			ConsumeWhitespace();

			if (variadic)
			{
				parameters.push_back(std::move(parameter));
				break;
			}
			if (m_Lexer.next_is(')'))
			{
				parameters.push_back(std::move(parameter));
				break;
			}
			if (m_Lexer.next_is("=") && optional)
			{
				AssertSpecific('=');
				ConsumeWhitespace();
				auto defaultValue = m_Lexer.consume_until([](auto ch)
														  { return std::isspace(ch) || ch == ',' || ch == ')'; });
				parameter.DefaultValue = std::move(std::string(defaultValue));
			}

			parameters.push_back(std::move(parameter));
			if (m_Lexer.next_is(')'))
				ConsumeWhitespace();

			AssertSpecific(',');
			ConsumeWhitespace();
		}

		return parameters;
	}

	Function Parser::ParseFunction(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface, bool isSpecial)
	{
		bool static_ = m_Lexer.consume_specific("static");
		if (static_)
			ConsumeWhitespace();

		auto returnType = ParseType();
		ConsumeWhitespace();
		auto name = m_Lexer.consume_until([](auto c)
										  { return std::isspace(c) || c == '('; });
		ConsumeWhitespace();
		AssertSpecific('(');
		auto parameters = ParseParameters();
		ConsumeWhitespace();
		AssertSpecific(')');
		ConsumeWhitespace();
		AssertSpecific(';');

		Function function{
			std::move(returnType),
			std::move(std::string(name)),
			std::move(parameters),
			std::move(extendedAttributes),
		};

		// "Defining a special operation with an identifier is equivalent to separating the special operation out into its own declaration without an identifier."
		if (!isSpecial || (isSpecial && name.empty()))
		{
			if (static_)
				interface.StaticFunctions.push_back(function);
			else
				interface.Functions.push_back(function);
		}

		return function;
	}

	void Parser::ParseConstructor(Interface& interface)
	{
		AssertString("constructor");
		ConsumeWhitespace();
		AssertSpecific('(');
		auto parameters = ParseParameters();
		ConsumeWhitespace();
		AssertSpecific(')');
		ConsumeWhitespace();
		AssertSpecific(';');

		interface.Constructors.push_back(Constructor{
			interface.Name,
			std::move(parameters)});
	}

	void Parser::ParseStringifier(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface)
	{
		AssertString("stringifier");
		ConsumeWhitespace();

		interface.HasStringifier = true;

		if (m_Lexer.next_is("readonly") || m_Lexer.next_is("attribute"))
		{
			ParseAttribute(extendedAttributes, interface);
			interface.StringifierAttribute = interface.Attributes.rbegin()->Name;
		}
		else
		{
			AssertSpecific(';');
		}
	}

	void Parser::ParseIterable(Interface& interface)
	{
		AssertString("iterable");
		AssertSpecific('<');
		auto firstType = ParseType();
		if (m_Lexer.next_is(','))
		{
			if (interface.SupportsIndexedProperties())
				ReportParsingError("Interfaces with a pair interator must not support indexed properties.", m_Filename, m_Input, m_Lexer.tell());

			AssertSpecific(',');
			ConsumeWhitespace();
			auto secondType = ParseType();
			interface.PairIteratorTypes = std::pair{std::move(firstType), std::move(secondType)};
		}
		else
		{
			if (!interface.SupportsIndexedProperties())
				ReportParsingError("Interfaces with an iterator must support indexed properties.", m_Filename, m_Input, m_Lexer.tell());

			interface.ValueIteratorType = std::move(firstType);
		}

		AssertSpecific('>');
		AssertSpecific(';');
	}

	void Parser::ParseGetter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface)
	{
		AssertString("getter");
		ConsumeWhitespace();
		auto function = ParseFunction(extendedAttributes, interface, true);

		if (function.Parameters.size() != 1)
			ReportParsingError("Named/indexed property getters must have exactly one parameter, got " + std::to_string(function.Parameters.size()) + ".", m_Filename, m_Input, m_Lexer.tell());

		auto& identifier = function.Parameters.front();

		if (identifier.Type->Nullable)
			ReportParsingError("Named/indexed property getter type must not be nullable.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Optional)
			ReportParsingError("Named/indexed property getters must not be optional.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Variadic)
			ReportParsingError("Named/indexed property getters must not be variadic.", m_Filename, m_Input, m_Lexer.tell());

		if (identifier.Type->Name == "DOMString")
		{
			if (interface.NamedPropertyGetter.has_value())
				ReportParsingError("An interface may only have one named property getter.", m_Filename, m_Input, m_Lexer.tell());

			interface.NamedPropertyGetter = std::move(function);
		}
		else if (identifier.Type->Name == "unsigned long")
		{
			if (interface.IndexedPropertyGetter.has_value())
				ReportParsingError("An interface may only have one indexed property getter.", m_Filename, m_Input, m_Lexer.tell());

			interface.IndexedPropertyGetter = std::move(function);
		}
		else
		{
			ReportParsingError("Named/indexed property getters must have a DOMString or unsigned long parameter, got " + identifier.Type->Name + ".", m_Filename, m_Input, m_Lexer.tell());
		}
	}

	void Parser::ParseSetter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface)
	{
		AssertString("setter");
		ConsumeWhitespace();
		auto function = ParseFunction(extendedAttributes, interface, true);

		if (function.Parameters.size() != 2)
			ReportParsingError("Named/indexed property setters must have exactly two parameters, got " + std::to_string(function.Parameters.size()) + ".", m_Filename, m_Input, m_Lexer.tell());

		auto& identifier = function.Parameters.front();

		if (identifier.Type->Nullable)
			ReportParsingError("Named/indexed property setter type must not be nullable.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Optional)
			ReportParsingError("Named/indexed property setters must not be optional.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Variadic)
			ReportParsingError("Named/indexed property setters must not be variadic.", m_Filename, m_Input, m_Lexer.tell());

		if (identifier.Type->Name == "DOMString")
		{
			if (interface.NamedPropertySetter.has_value())
				ReportParsingError("An interface may only have one named property setter.", m_Filename, m_Input, m_Lexer.tell());

			interface.NamedPropertySetter = std::move(function);
		}
		else if (identifier.Type->Name == "unsigned long")
		{
			if (interface.IndexedPropertySetter.has_value())
				ReportParsingError("An interface may only have one indexed property setter.", m_Filename, m_Input, m_Lexer.tell());

			interface.IndexedPropertySetter = std::move(function);
		}
		else
		{
			ReportParsingError("Named/indexed property setters must have a DOMString or unsigned long parameter, got " + identifier.Type->Name + ".", m_Filename, m_Input, m_Lexer.tell());
		}
	}

	void Parser::ParseDeleter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface)
	{
		AssertString("deleter");
		ConsumeWhitespace();
		auto functon = ParseFunction(extendedAttributes, interface, true);

		if (functon.Parameters.size() != 1)
			ReportParsingError("Named/indexed property deleters must have exactly one parameter, got " + std::to_string(functon.Parameters.size()) + ".", m_Filename, m_Input, m_Lexer.tell());

		auto& identifier = functon.Parameters.front();

		if (identifier.Type->Nullable)
			ReportParsingError("Named/indexed property deleter type must not be nullable.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Optional)
			ReportParsingError("Named/indexed property deleters must not be optional.", m_Filename, m_Input, m_Lexer.tell());
		if (identifier.Variadic)
			ReportParsingError("Named/indexed property deleters must not be variadic.", m_Filename, m_Input, m_Lexer.tell());

		if (identifier.Type->Name == "DOMString")
		{
			if (interface.NamedPropertyDeleter.has_value())
				ReportParsingError("An interface may only have one named property deleter.", m_Filename, m_Input, m_Lexer.tell());

			interface.NamedPropertyDeleter = std::move(functon);
		}
		else
		{
			ReportParsingError("Named/indexed property deleters must have a DOMString or unsigned long parameter, got " + identifier.Type->Name + ".", m_Filename, m_Input, m_Lexer.tell());
		}
	}

	void Parser::ParseInterface(Interface& interface)
	{
		ConsumeWhitespace();
		interface.Name = m_Lexer.consume_until([](auto ch)
											   { return std::isspace(ch); });
		ConsumeWhitespace();
		if (m_Lexer.consume_specific(':'))
		{
			ConsumeWhitespace();
			interface.ParentName = m_Lexer.consume_until([](auto ch)
														 { return std::isspace(ch); });
			ConsumeWhitespace();
		}

		AssertSpecific('{');

		for (;;)
		{
			std::unordered_map<std::string, std::string> extendedAttributes;

			ConsumeWhitespace();

			if (m_Lexer.consume_specific('}'))
			{
				ConsumeWhitespace();
				AssertSpecific(';');
				break;
			}

			if (m_Lexer.consume_specific('['))
			{
				extendedAttributes = ParseExtendedAttributes();
				if (!interface.HasUnscopableMember && extendedAttributes.contains("Unscopable"))
					interface.HasUnscopableMember = true;
			}

			if (m_Lexer.next_is("constructor"))
			{
				ParseConstructor(interface);
				continue;
			}

			if (m_Lexer.next_is("const"))
			{
				ParseConstant(interface);
				continue;
			}

			if (m_Lexer.next_is("stringifier"))
			{
				ParseStringifier(extendedAttributes, interface);
				continue;
			}

			if (m_Lexer.next_is("iterable"))
			{
				ParseIterable(interface);
				continue;
			}

			if (m_Lexer.next_is("readonly") || m_Lexer.next_is("attribute"))
			{
				ParseAttribute(extendedAttributes, interface);
				continue;
			}

			if (m_Lexer.next_is("getter"))
			{
				ParseGetter(extendedAttributes, interface);
				continue;
			}

			if (m_Lexer.next_is("setter"))
			{
				ParseSetter(extendedAttributes, interface);
				continue;
			}

			if (m_Lexer.next_is("deleter"))
			{
				ParseDeleter(extendedAttributes, interface);
				continue;
			}

			ParseFunction(extendedAttributes, interface);
		}

		interface.WrapperClass = interface.Name + "Wrapper";
		interface.WrapperBaseClass = interface.ParentName + "Wrapper";
		interface.ConstructorClass = interface.Name + "Constructor";
		interface.PrototypeClass = interface.Name + "Prototype";
		if (interface.ParentName.empty())
			interface.PrototypeBaseClass = "ObjectPrototype";
		else
			interface.PrototypeBaseClass = interface.ParentName + "Prototype";

		ConsumeWhitespace();
	}

	void Parser::ParseEnum(Interface& interface)
	{
		AssertString("enum");
		ConsumeWhitespace();

		Enum enumeration{};

		auto name = m_Lexer.consume_until([](auto ch)
										  { return std::isspace(ch); });
		ConsumeWhitespace();

		AssertSpecific('{');

		bool first = true;
		for (; !m_Lexer.is_eof();)
		{
			ConsumeWhitespace();

			if (m_Lexer.next_is('}'))
				break;

			if (!first)
			{
				AssertSpecific(',');
				ConsumeWhitespace();
			}

			AssertSpecific('"');
			auto string = m_Lexer.consume_until('"');
			AssertSpecific('"');
			ConsumeWhitespace();

			if (enumeration.Values.find(std::string(string)) != enumeration.Values.end())
				ReportParsingError("Enumeration value " + std::string(string) + " is already defined.", m_Filename, m_Input, m_Lexer.tell());
			else
				enumeration.Values.insert(std::string(string));

			if (first)
				enumeration.FirstMember = std::move(string);

			first = false;
		}

		ConsumeWhitespace();
		AssertSpecific('}');
		AssertSpecific(';');

		std::unordered_set<std::string> seen_names;
		for (auto& entry : enumeration.Values)
			enumeration.TranslatedCppNames.emplace(entry, ConvertEnumValueToCppEnumMember(entry, seen_names));

		interface.Enums.emplace(name, std::move(enumeration));
		ConsumeWhitespace();
	}

	void Parser::ParseDictionary(Interface& interface)
	{
		AssertString("dictionary");
		ConsumeWhitespace();

		Dict dictionary{};

		auto name = m_Lexer.consume_until([](auto ch)
										  { return std::isspace(ch); });
		ConsumeWhitespace();

		if (m_Lexer.consume_specific(':'))
		{
			ConsumeWhitespace();
			dictionary.ParentName = m_Lexer.consume_until([](auto ch)
														  { return std::isspace(ch); });
			ConsumeWhitespace();
		}
		AssertSpecific('{');

		for (;;)
		{
			ConsumeWhitespace();

			if (m_Lexer.consume_specific('}'))
			{
				ConsumeWhitespace();
				AssertSpecific(';');
				break;
			}

			bool required = false;
			std::unordered_map<std::string, std::string> extendedAttributes;

			if (m_Lexer.consume_specific("required"))
			{
				required = true;
				ConsumeWhitespace();
				if (m_Lexer.consume_specific('['))
					extendedAttributes = ParseExtendedAttributes();
			}

			auto type = ParseType();
			ConsumeWhitespace();

			auto name = m_Lexer.consume_until([](auto ch)
											  { return std::isspace(ch) || ch == ';'; });
			ConsumeWhitespace();

			std::optional<std::string> defaultValue;

			if (m_Lexer.consume_specific('='))
			{
				assert(!required);
				ConsumeWhitespace();
				defaultValue = m_Lexer.consume_until([](auto ch)
													 { return std::isspace(ch) || ch == ';'; });
				ConsumeWhitespace();
			}

			AssertSpecific(';');

			DictMember member{
				required,
				std::move(type),
				std::string(name),
				std::move(extendedAttributes),
				std::optional<std::string>(std::move(defaultValue))};

			dictionary.Members.emplace_back(std::move(member));
		}

		// Dictionary members need to be in lexicographical order.
		qsort(dictionary.Members.data(), dictionary.Members.size(), sizeof(DictMember), [](const void* a, const void* b)
			  {
				  auto& aa = *reinterpret_cast<const DictMember*>(a);
				  auto& bb = *reinterpret_cast<const DictMember*>(b);
				  return strcmp(aa.Name.c_str(), bb.Name.c_str()); });

		interface.Dictionaries.emplace(name, std::move(dictionary));
		ConsumeWhitespace();
	}

	void Parser::ParseInterfaceMixin(Interface& interface)
	{
		auto mixinInterface = std::make_shared<Interface>();
		mixinInterface->ModuleOwnPath = interface.ModuleOwnPath;
		mixinInterface->IsMixin = true;

		AssertString("interface");
		ConsumeWhitespace();
		AssertString("mixin");
		auto offset = m_Lexer.tell();

		ParseInterface(*mixinInterface);

		if (!mixinInterface->ParentName.empty())
			ReportParsingError("Mixin interfaces cannot have a parent.", m_Filename, m_Input, offset);

		auto name = mixinInterface->Name;

		interface.Mixins.emplace(std::string(name), std::move(mixinInterface));
	}

	void Parser::ParseNonInterfaceEntities(bool allowInterface, Interface& interface)
	{
		while (!m_Lexer.is_eof())
		{
			if (m_Lexer.next_is("dictionay"))
				ParseDictionary(interface);
			else if (m_Lexer.next_is("enum"))
				ParseEnum(interface);
			else if (m_Lexer.next_is("interface mixin"))
				ParseInterfaceMixin(interface);
			else if ((allowInterface && !m_Lexer.next_is("interface")) || !allowInterface)
			{
				auto currentOffset = m_Lexer.tell();
				auto name = m_Lexer.consume_until([](auto ch)
												  { return std::isspace(ch); });
				ConsumeWhitespace();
				if (m_Lexer.consume_specific("includes"))
				{
					ConsumeWhitespace();
					auto mixinName = m_Lexer.consume_until([](auto ch)
														   { return std::isspace(ch) || ch == ';'; });

					auto nameStr = std::string(name);
					auto mapEntry = interface.IncludedMixins.find(nameStr);
					if (mapEntry != interface.IncludedMixins.end())
						mapEntry->second.emplace(nameStr);
					else
						interface.IncludedMixins.emplace(mixinName, std::unordered_set<std::string>{nameStr});

					ConsumeWhitespace();
					AssertSpecific(';');
					ConsumeWhitespace();
				}
				else
				{
					ReportParsingError("Expected 'enum' or 'dictionary'.", m_Filename, m_Input, currentOffset);
				}
			}
			else
				break;
		}
	}

	std::shared_ptr<Interface> Parser::Parse()
	{
		auto thisModule = std::filesystem::canonical(m_Filename).string();
		s_AllImportedPaths.emplace(thisModule);

		auto interface = std::make_shared<Interface>();
		interface->ModuleOwnPath = thisModule;

		std::vector<std::shared_ptr<Interface>> imports;
		while (m_Lexer.consume_specific("#import"))
		{
			ConsumeWhitespace();
			AssertSpecific('<');
			auto path = m_Lexer.consume_until('>');
			m_Lexer.ignore();

			auto maybeInterface = ResolveImport(path);
			if (maybeInterface)
			{
				for (auto& entry : maybeInterface.value()->ImportedPaths)
					s_AllImportedPaths.emplace(entry);
				imports.emplace_back(std::move(maybeInterface.value()));
			}

			ConsumeWhitespace();
		}

		interface->ImportedPaths = s_AllImportedPaths;

		if (m_Lexer.consume_specific('['))
			interface->ExtendedAttributes = ParseExtendedAttributes();

		ParseNonInterfaceEntities(true, *interface);

		if (m_Lexer.consume_specific("interface"))
			ParseInterface(*interface);

		ParseNonInterfaceEntities(false, *interface);

		for (auto& import : imports)
		{
			// FIXME: Instead of copying every imported entity into the current interface, we should query imports directly.
			for (auto& dict : import->Dictionaries)
				interface->Dictionaries.emplace(dict.first, dict.second);

			for (auto& enumEntry : import->Enums)
			{
				auto enumCopy = enumEntry.second;
				enumCopy.IsOriginalDefinition = false;
				interface->Enums.emplace(enumEntry.first, std::move(enumCopy));
			}

			for (auto& mixin : import->Mixins)
			{
				if (interface->Mixins.contains(mixin.first))
					ReportParsingError("Mixin '" + mixin.first + "' was already defined in '" + mixin.second->ModuleOwnPath + "'.", m_Filename, m_Input, m_Lexer.tell());
				interface->Mixins.emplace(mixin.first, mixin.second);
			}
		}

		// Resolve Mixins.
		if (auto it = interface->IncludedMixins.find(interface->Name); it != interface->IncludedMixins.end())
		{
			for (auto& entry : it->second)
			{
				auto mixinIterator = interface->Mixins.find(entry);
				if (mixinIterator == interface->Mixins.end())
					ReportParsingError("Mixin '" + entry + "' was never defined.", m_Filename, m_Input, m_Lexer.tell());

				auto& mixin = mixinIterator->second;
				interface->Attributes.insert(interface->Attributes.end(), mixin->Attributes.begin(), mixin->Attributes.end());
				interface->Constants.insert(interface->Constants.end(), mixin->Constants.begin(), mixin->Constants.end());
				interface->Functions.insert(interface->Functions.end(), mixin->Functions.begin(), mixin->Functions.end());
				interface->StaticFunctions.insert(interface->StaticFunctions.end(), mixin->StaticFunctions.begin(), mixin->StaticFunctions.end());
				if (interface->HasStringifier && mixin->HasStringifier)
					ReportParsingError("Both interface '" + interface->Name + "' and mixin '" + mixin->Name + "' defined a stringifier.", m_Filename, m_Input, m_Lexer.tell());

				if (mixin->HasStringifier)
				{
					interface->HasStringifier = true;
					interface->StringifierAttribute = mixin->StringifierAttribute;
				}
			}
		}

		interface->ImportedModules = std::move(imports);
		return interface;
	}

	Parser::Parser(std::string_view filename, std::string_view contents, std::string_view importBasePath)
		: m_ImportBasePath(std::move(importBasePath)), m_Filename(std::move(filename)), m_Input(contents), m_Lexer(contents)
	{
	}

}