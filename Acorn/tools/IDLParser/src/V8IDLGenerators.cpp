/**
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

// FIXME move to inja::Environment to precompile templates
// FIXME move this project to a subproject or similar to reduce reconfigure complexity.
// FIXME change parser to emit more favourable types for adding to SourceGenerator
// FIXME Currently we assume that a wrapped function is in Acorn (Pascal case) style. This is not true for wrapped functions from other classes (i.e. glm::vec)

#include "IDLTypes.h"
#include "SourceGenerator.h"

#include <fmt/format.h>
#include <inja/inja.hpp>

#include <fstream>
#include <iostream>
#include <queue>
#include <utility>

std::vector<std::string_view> s_HeaderSearchPaths;

namespace Acorn::IDL
{
	using namespace std::literals::string_view_literals; // To allow ""sv construction

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

	static std::string ToCppType(std::shared_ptr<Type> ty, const nlohmann::json& mappingData)
	{
		if (ty->IsInteger())
		{
			return "uint32_t";
		}
		if (ty->IsNumeric())
		{
			return "double";
		}
		if (ty->IsString())
		{
			return "std::string";
		}
		if (ty->Name == "boolean")
		{
			return "bool";
		}

		for (auto& [key, value] : mappingData.items())
		{

			if (ty->Name == key)
			{
				return mappingData.at(key).get<std::string>();
			}
		}

		return ty->Name;
	}

	static void AddEnums(inja::json& data, const Interface& interface)
	{
		data["enums"] = inja::json::array();

		for (const auto& enumeration : interface.Enums)
		{
			if (!enumeration.second.IsOriginalDefinition)
				continue;

			// FIXME: Use translated Cpp Names...
			inja::json enumerationData;
			enumerationData["name"]	  = enumeration.first;
			enumerationData["values"] = inja::json::array();
			for (const auto& value : enumeration.second.Values)
			{
				enumerationData["values"].push_back(value);
			}

			data["enums"].push_back(enumerationData);
		}
	}

	void GenerateHeader(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath, nlohmann::json mappingData)
	{
		SourceGenerator generator(std::move(templateBasePath));

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		// EmitIncludesForAllImports(interface, generator, true);
		//		generator.Add("name", interface.Name);
		generator.Add("interface_name", interface.Name);
		generator.Add("wrapper_name", interface.Name + "Wrapper");


		if (interface.ExtendedAttributes.contains("Bind"))
		{
			generator.Add("name", interface.ExtendedAttributes.at("Bind"));
			// TODO figure out include paths
		}
		else
		{
			generator.Add("name", interface.Name);
		}
		generator.Add("fully_qualified_name", interface.FullyQualifiedName);
		generator.Add("wrapper_class", interface.WrapperClass);
		generator.Add("wrapper_base_class", interface.WrapperBaseClass);

		inja::json data;
		AddEnums(data, interface);

		generator.Add(data);

		if (interface.ExtendedAttributes.contains("Extend"))
		{
			generator.AppendFile("ExtendHeader.tpl");
		}
		else
		{
			generator.AppendFile("WrapperHeader.tpl");
		}

		std::ofstream out(outPath.string());
		out << generator.Generate();
	}

	void GenerateImplementation(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath, nlohmann::json mappingData)
	{
		SourceGenerator generator(std::move(templateBasePath));

		// TODO: ExtendedAttributes CustomCppClass=<classname> to wrap external, non-Acorn classes (i.e. glm)

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		inja::json data;
		data["wrapper_name"]   = std::string(interface.Name + "Wrapper");
		data["interface_name"] = interface.Name;

		AddEnums(data, interface);

		if (interface.ExtendedAttributes.contains("Bind"))
		{
			data["name"]		 = interface.ExtendedAttributes.at("Bind");
			data["typedef_name"] = interface.ExtendedAttributes.at("Bind");
			// TODO figure out include paths
			// TODO replace names with typedef name
		}
		else
		{
			data["name"]		 = interface.Name;
			data["typedef_name"] = false;
		}

		data["constructors"] = inja::json::array();

		for (const auto& ctor : interface.Constructors)
		{
			inja::json ctorData;
			ctorData["cpp_args"] = inja::json::array();
			for (const auto& arg : ctor.Parameters)
			{
				ctorData["cpp_args"].push_back(ToCppType(arg.Type, mappingData));
			}
			data["constructors"].push_back(ctorData);
		}

		if (interface.WrapperBaseClass != "Wrapper")
		{
			data["inherit"] = interface.WrapperBaseClass;
		}
		else
		{
			data["inherit"] = false;
		}

		data["properties"] = inja::json::array();

		for (const auto& prop : interface.Attributes)
		{
			inja::json propData;
			propData["name"]	 = prop.Name;
			propData["readonly"] = prop.ReadOnly ? "true" : "false";
			// TODO GetterNameCallback ExtendedAttributes,....
			propData["virtual"] = prop.ReadOnly;
			data["properties"].push_back(propData);
		}

		data["methods"] = inja::json::array();

		for (const auto& method : interface.Functions)
		{
			if (method.Name.empty())
				continue;

			inja::json methodData;
			methodData["name"]		  = method.Name;
			methodData["return_type"] = ToCppType(method.ReturnType, mappingData);

			if (method.ExtendedAttributes.contains("Bind"))
			{
				methodData["cpp_name"] = method.ExtendedAttributes.at("Bind");
			}
			else
			{
				methodData["cpp_name"] = ToPascalCase(method.Name);
			}

			if (method.ExtendedAttributes.contains("External"))
			{
				methodData["external"] = method.ExtendedAttributes.at("External");
			}
			else
			{
				methodData["external"] = false;
			}

			methodData["args"] = inja::json::array();
			for (const auto& arg : method.Parameters)
			{
				inja::json argData;
				argData["name"]		= arg.Name;
				argData["type"]		= ToCppType(arg.Type, mappingData);
				argData["optional"] = arg.Optional; // TODO etc.
				methodData["args"].push_back(argData);
			}

			data["methods"].push_back(methodData);
		}

		if (interface.IndexedPropertyGetter)
		{
			// TODO do we care about the function definition?
			// FIXME We should definitely assert that the function is a valid indexed getter...
			data["indexed_property_getter"] = true;
		}
		else
		{
			data["indexed_property_getter"] = false;
		}

		if (interface.IndexedPropertySetter)
		{
			// FIXME We should definitely assert that the function is a valid indexed setter...
			data["indexed_property_setter"] = { { "type", ToCppType(interface.IndexedPropertySetter->Parameters[1].Type, mappingData) } };
		}
		else
		{
			data["indexed_property_setter"] = false;
		}

		if (interface.NamedPropertyGetter)
		{
			// TODO do we care about the function definition?
			// FIXME We should definitely assert that the function is a valid named getter...
			data["named_property_getter"] = true;
		}
		else
		{
			data["named_property_getter"] = false;
		}

		if (interface.NamedPropertySetter)
		{
			// TODO do we care about the function definition?
			// FIXME We should definitely assert that the function is a valid named setter...
			data["named_property_setter"] = { { "type", ToCppType(interface.IndexedPropertyGetter->Parameters[1].Type, mappingData) } };
		}
		else
		{
			data["named_property_setter"] = false;
		}

		if (interface.HasStringifier)
		{
			data["stringifier"] = true;
		}
		else
		{
			data["stringifier"] = false;
		}

		// FIXME: for now setter without getter does not work,
		// TODO: Deleter

		generator.Add(data);

		// generator.Append(WrapperImplementation);
		if (interface.ExtendedAttributes.contains("Extend"))
		{
			generator.AppendFile("ExtendImplementation.tpl");
		}
		else
		{
			generator.AppendFile("WrapperImplementation.tpl");
		}

		std::ofstream outFile(outPath.string());
		outFile << generator.Generate();
	}

} // namespace Acorn::IDL
