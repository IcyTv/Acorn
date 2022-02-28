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

#include <inja/inja.hpp>

#include <iostream>
#include <queue>
#include <fstream>
#include <utility>

std::vector<std::string_view> s_HeaderSearchPaths;

namespace Acorn::IDL
{
	using namespace std::literals::string_view_literals; // To allow ""sv construction

	static bool IsWrappableType(std::shared_ptr<Type> type)
	{
		return false;
	}

	static auto ToV8Type(const std::shared_ptr<Type>& type) -> std::string
	{
		if (type->IsNumeric())
		{
			return "Number";
		}
		if (type->IsString())
		{
			return "String";
		}
		if (type->Name == "void")
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

	void GenerateHeader(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath)
	{
		SourceGenerator generator(std::move(templateBasePath));

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		// EmitIncludesForAllImports(interface, generator, true);
		generator.Add("name", interface.Name);
		generator.Add("fully_qualified_name", interface.FullyQualifiedName);
		generator.Add("wrapper_class", interface.WrapperClass);
		generator.Add("wrapper_base_class", interface.WrapperBaseClass);

		//if (interface.WrapperBaseClass == "Wrapper")
		//	GenerateIncludeForWrapper(generator, interface.WrapperClass);

		// ==================
		// == Enumerations ==
		// ==================

        inja::json data;
        data["enums"] = inja::json::array();

        for (const auto& enumeration : interface.Enums)
        {
            if(!enumeration.second.IsOriginalDefinition)
                continue;
            inja::json enumerationData;
            enumerationData["name"] = enumeration.first;
            enumerationData["values"] = inja::json::array();
            for (const auto& value : enumeration.second.Values)
            {
                enumerationData["values"].push_back(value);
            }

            data["enums"].push_back(enumerationData);
        }

        generator.Add(data);
        generator.AppendFile("WrapperHeader.tpl");

		std::ofstream out(outPath.string());
		out << generator.Generate();
	}

	void GenerateImplementation(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath)
	{
		SourceGenerator generator(std::move(templateBasePath));

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
				{ "v8_type", ToV8Type(attribute.Type) },
			});
		}

		for (auto& func : interface.Functions)
		{
			if(func.Name.empty())
				continue;

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

			data["methods"].push_back(methodData);
		}

		if(interface.IndexedPropertyGetter)
		{
			auto getter = *interface.IndexedPropertyGetter;
			// https://webidl.spec.whatwg.org/#idl-indexed-properties
			// An indexed property must be of signature: (unsinged long index) -> T
			assert(getter.Parameters.size() == 1);
			assert(getter.Parameters[0].Type->Name == "unsigned long");

			// This also means, we don't neeed to pass the arguments to the template,
			// since they are predefined
			data["indexed_property_getter"] = {
				{ "return_type", getter.ReturnType->Name },
				{ "return_v8_type", ToV8Type(getter.ReturnType) },
			};
		} else {
			data["indexed_property_getter"] = false;
		}

		if(interface.IndexedPropertySetter)
		{
			auto setter = *interface.IndexedPropertySetter;
			// https://webidl.spec.whatwg.org/#idl-indexed-properties
			// An indexed property must be of signature: (unsinged long index, T value) -> W
			assert(setter.Parameters.size() == 2);
			assert(setter.Parameters[0].Type->Name == "unsigned long");

			data["indexed_property_setter"] = {
				{ "return_type", setter.ReturnType->Name },
				{ "return_v8_type", ToV8Type(setter.ReturnType) },
				{ "value_type", setter.Parameters[1].Type->Name },
				{ "value_v8_type", ToV8Type(setter.Parameters[1].Type) },
			};
		} else {
			data["indexed_property_setter"] = false;
		}

		generator.Add(data);

		// generator.Append(WrapperImplementation);
		generator.AppendFile("WrapperImplementation.tpl");

		std::ofstream outFile(outPath.string());
		outFile << generator.Generate();
	}

} // namespace Acorn::IDL
