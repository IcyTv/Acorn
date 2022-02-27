/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2021, Luke Wilde <lukew@serenityos.org>
 * Copyright (c) 2022, Ali Mohammad Pur <mpfard@serenityos.org>
 * Copyright (c) 2022, Michael Finger <michael.finger@icytv.de>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

// FIXME move to inja::Environment to precompile templates
// FIXME move this project to a subproject or similar to reduce reconfigure complexity.
// FIXME change parser to emit more favourable types for adding to SourceGenerator

#include "IDLTypes.h"
#include "SourceGenerator.h"

#include "EnumHeader.h"
#include "WrapperHeader.h"
#include "WrapperImplementation.h"

#include <boost/algorithm/string/replace.hpp>
#include <inja/inja.hpp>

#include <filesystem>
#include <iostream>
#include <queue>

std::vector<std::string_view> s_HeaderSearchPaths;

namespace Acorn::IDL
{
	using namespace std::literals::string_view_literals; // To allow ""sv construction

	static bool IsWrappableType(std::shared_ptr<Type> type)
	{
		return false;
	}

	static std::string ToV8Type(const std::shared_ptr<Type>& type)
	{
		if (type->IsNumeric())
		{
			return "Number";
		}
		else if (type->IsString())
		{
			return "String";
		}
		else if (type->Name == "void")
		{
			return "Undefined";
		}
		throw std::runtime_error("Unsupported type " + type->Name);
	}

	constexpr std::string_view SequenceStorageTypeToCppStorageName(SequenceStorageType type)
	{
		switch (type)
		{
		case SequenceStorageType::Vector: return "std::vector"sv;
		case SequenceStorageType::MarkedVector:
			// static_assert(false, "MarkedVector is not supported
			// yet");
			assert(false);
		default:
			// static_assert(false, "Unknown SequenceStorageType");
			assert(false);
			return "";
		}
	}

	static CppType From(std::shared_ptr<Type> type, const Interface& interface)
	{
		if (IsWrappableType(type))
		{
			assert(false || !"Wrappable types are not supported yet");
		}

		if (type->IsString())
			return {
				.Name			 = "std::string",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		if (type->Name == "double" && !type->Nullable)
		{
			return {
				.Name			 = "double",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "bool" && !type->Nullable)
		{
			return {
				.Name			 = "bool",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "unsigned long" && !type->Nullable)
		{
			return {
				.Name			 = "uint32_t",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "unsigned short" && !type->Nullable)
		{
			return {
				.Name			 = "uint16_t",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "long" && !type->Nullable)
		{
			return {
				.Name			 = "int32_t",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "any")
		{
			assert(false || !"any is not supported yet");
		}
		if (type->Name == "sequence")
		{
			auto parameterizedType = VerifyCast<ParameterizedType>(type);
			auto sequenceType	   = parameterizedType->Parameters.front();
			assert(sequenceType);
			auto sequenceCppType = CppType::From(sequenceType, interface);
			auto storageTypeName = SequenceStorageTypeToCppStorageName(sequenceCppType.SequenceStorage);

			if (sequenceCppType.SequenceStorage == SequenceStorageType::MarkedVector)
			{
				assert(false || !"MarkedVector is not supported yet");
			}

			return {
				.Name			 = std::string(storageTypeName) + "<" + sequenceCppType.Name + ">",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}
		if (type->Name == "record")
		{
			auto parameterizedType = VerifyCast<ParameterizedType>(type);
			auto recordKeyType	   = parameterizedType->Parameters.front();
			assert(recordKeyType);
			auto recordValueType = parameterizedType->Parameters.back();
			assert(recordValueType);
			auto recordKeyCppType	= CppType::From(recordKeyType, interface);
			auto recordValueCppType = CppType::From(recordValueType, interface);

			return {
				.Name			 = "std::unordered_map<" + recordKeyCppType.Name + ", " + recordValueCppType.Name + ">",
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}

		if (Is<UnionType>(type))
		{
			auto unionType = VerifyCast<UnionType>(type);
			return {
				.Name			 = unionType->ToVariant(interface),
				.SequenceStorage = SequenceStorageType::Vector,
			};
		}

		if (!type->Nullable)
		{
			for (auto& dictionary : interface.Dictionaries)
			{
				if (type->Name == dictionary.first)
					return {
						.Name			 = type->Name,
						.SequenceStorage = SequenceStorageType::Vector,
					};
			}
		}

		assert(false || !"Unknown type");
	}

	static std::string MakeInputAcceptableCpp(const std::string& input)
	{
		if (IsOneOf(input, "class", "template", "for", "default", "char", "namespace", "delete"))
		{
			return input + "_";
		}

		std::string output(input);
		std::replace(output.begin(), output.end(), '-', '_');
		return output;
	}

	static void GenerateIncludeForWrapper(SourceGenerator& generator, const std::string& wrapperName)
	{
		generator.Add("wrapperClass", wrapperName);
		// FIXME: These may or may not exist, because REASONS.
		// ^^^^
		// This Fixme is copied straight from Serenity... What the fuck
		// does he mean? What Reasons? What??
		generator.Append(R"~~~(
#if __has_include("{{ acorn_root }}/Bindings/{{ wrapperClass }}.h")
	#include "{{acorn_root}}/Bindings/{{ wrapperClass }}.h"
#endif
#if __has_include("{{ acorn_root }}/Bindings/{{ wrapperClass }}Factory.h")
	#include "{{ acorn_root }}/Bindings/{{ wrapperClass }}Factory.h"
#endif
		)~~~"sv);
	}

	static void GenerateIncludeFor(SourceGenerator& generator, const std::string& path)
	{
		auto pathStr = path;
		for (auto& searchPath : s_HeaderSearchPaths)
		{
			if (!path.starts_with(searchPath))
				continue;

			auto relativePath = std::filesystem::relative(path, searchPath).string();
			// What is this?
			if (relativePath.length() < pathStr.length())
				pathStr = relativePath;
		}

		std::filesystem::path fspath(pathStr);
		generator.Add("include.path", fspath.parent_path().string() + "/" + fspath.filename().string() + ".h");
		generator.Append(R"~~~(
#include <{{ include.path }}>
		)~~~");
	}

	static void EmitIncludesForAllImports(const Interface& interface, SourceGenerator& generator, bool isHeader, bool isIterator = false)
	{
		std::queue<const Interface*> interfaces;
		std::unordered_set<std::string> pathsImported;

		if (isHeader)
			pathsImported.emplace(interface.ModuleOwnPath);

		interfaces.push(&interface);

		while (!interfaces.empty())
		{
			const Interface* interface = interfaces.front();
			interfaces.pop();

			if (std::find(pathsImported.begin(), pathsImported.end(), interface->ModuleOwnPath) != pathsImported.end())
				continue;

			pathsImported.emplace(interface->ModuleOwnPath);
			for (auto& importedInterface : interface->ImportedModules)
			{
				if (std::find(pathsImported.begin(), pathsImported.end(), importedInterface->ModuleOwnPath) != pathsImported.end())
					interfaces.push(importedInterface.get());
			}

			GenerateIncludeFor(generator, interface->ModuleOwnPath);

			if (isIterator)
			{
				// auto iterName		 = interface->Name + "Iterator";
				// std::string iterPath = interface->FullyQualifiedName + "Iterator";
				// boost::replace_all(iterPath, "::", "/");
				// GenerateIncludeForIterator(generator, iterPath, iterName);
				assert(false || !"Not implemented");
			}

			if (interface->WrapperClass != "Wrapper")
				GenerateIncludeForWrapper(generator, interface->WrapperClass);
		}
	}

	static bool ShouldEmitWrapperFactory(const Interface& interface)
	{
		// TODO
		return true;
	}

	template <typename ParameterType>
	static void GenerateToCpp(
		SourceGenerator& generator, ParameterType& parameter, const std::string& jsName, const std::string& jsSuffix, const std::string& cppName, const Interface& interface,
		bool legacyNullToEmptyString = false, bool optional = false, std::optional<std::string> optionalDefault = std::nullopt, bool variadic = false, size_t recursionDepth = 0,
		bool usedAsArgument = false
	)
	{
		auto acceptableName = MakeInputAcceptableCpp(cppName);
		generator.Add("cpp_name", acceptableName);
		generator.Add("js_name", jsName);
		generator.Add("js_suffix", jsSuffix);
		generator.Add("legacy_null_to_empty_string", legacyNullToEmptyString ? "true" : "false");
		generator.Add("parameter.type.name", parameter.Type->Name);
		generator.Add("wrapper_name", parameter.Type->Name + "Wrapper");

		if (optionalDefault.has_value())
			generator.Add("parameter.optional_default_value", optionalDefault.value());
	}

	void GenerateHeader(const Interface& interface)
	{
		SourceGenerator generator;

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		generator.Append(R"~~~(
/**
 * This file is generated from {{ idl_filename }}. Do not edit directly, as any changes will be lost.
 *
 * Copyright (c) 2022 Michael Finger
 * 
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 * 
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "{{ acorn_root }}/Wrapper.h"
#include "Acorn/Core.h"
)~~~");

		// for (auto& path : interface->I)
		// {
		// 	GenerateIncludeFor(generator, path);
		// }

		EmitIncludesForAllImports(interface, generator, true);
		generator.Add("name", interface.Name);
		generator.Add("fully_qualified_name", interface.FullyQualifiedName);
		generator.Add("wrapper_class", interface.WrapperClass);
		generator.Add("wrapper_base_class", interface.WrapperBaseClass);

		if (interface.WrapperBaseClass == "Wrapper")
			GenerateIncludeForWrapper(generator, interface.WrapperClass);

		generator.Append(R"~~~(
namespace Acorn::Scripting::V8
{
	class {{ wrapper_class }}: public {{ wrapper_base_class }}
	{
	public:

		virtual void Bind(v8::Local<v8::Object> object) override;

		)~~~");

		generator.Append(R"~~~(
	};
		)~~~");

		// ==================
		// == Enumerations ==
		// ==================

		for (auto& it : interface.Enums)
		{
			if (!it.second.IsOriginalDefinition)
				continue;

			inja::json enumData;
			enumData["name"]	= it.first;
			enumData["entries"] = {};
			for (auto& entry : it.second.TranslatedCppNames)
			{
				enumData["entries"].push_back({ { "name", entry.second }, { "string", entry.first } });
			}
			generator.Add(enumData);
			generator.Append(EnumHeader);
		}

		if (ShouldEmitWrapperFactory(interface))
		{
			generator.Append(R"~~~(
	@wrapper_class@* Wrap(v8::Local<v8::Object> global, @fully_qualified_name@&);
		)~~~");
		}

		generator.Append(R"~~~(
} // namespace Acorn::Scripting::V8
)~~~");

		std::cout << "===========\nHeader:\n========" << std::endl;
		std::cout << generator.Generate() << std::endl;
	}

	void GenerateImplementation(const Interface& interface)
	{
		SourceGenerator generator;

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		generator.Add("name", interface.Name);

		// EmitIncludesForAllImports(interface, generator, false);

		auto& constructor = interface.Constructors[0];

		inja::json data;
		data["constructor_args"] = {};
		data["attributes"]		 = {};
		data["methods"]			 = {};

		for (auto& arg : constructor.Parameters)
		{
			std::cout << arg.Name << ": " << arg.Type->Name << std::endl;
			data["constructor_args"].push_back({
				{ "type", arg.Type->Name },
				{ "v8_type", ToV8Type(arg.Type) },
				{ "name", arg.Name },
			});
		}

		for (auto& attribute : interface.Attributes)
		{
			data["attributes"].push_back({
				{ "name", attribute.Name },
				{ "type", attribute.Type->Name },
			});
		}

		for (auto& func : interface.Functions)
		{
			inja::json methodData;
			methodData["name"]			 = func.Name;
			methodData["args"]			 = inja::json::array();
			methodData["return_type"]	 = func.ReturnType->Name;
			methodData["return_v8_type"] = ToV8Type(func.ReturnType);

			for (auto& arg : func.Parameters)
			{
				inja::json argData;
				argData["type"]	   = arg.Type->Name;
				argData["v8_type"] = ToV8Type(arg.Type);
				argData["name"]	   = arg.Name;

				methodData["args"].push_back(argData);
			}

			std::cout << "Method: " << methodData << std::endl;

			data["methods"].push_back(methodData);
		}

		generator.Add(data);

		// generator.Append(WrapperImplementation);
		generator.AppendFile("WrapperImplementation.tpl");

		std::cout << "===========\nImplementation:\n========" << std::endl;
		std::cout << generator.Generate() << std::endl;
	}

	void GeneratePrototypeHeader(Interface const&) {}
	void GeneratePrototypeImplementation(Interface const&) {}
	void GenerateIteratorPrototypeHeader(Interface const&) {}
	void GenerateIteratorPrototypeImplementation(Interface const&) {}
	void GenerateIteratorHeader(Interface const&) {}
	void GenerateIteratorImplementation(Interface const&) {}

} // namespace Acorn::IDL
