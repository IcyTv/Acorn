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

	static std::string ToCppType(std::shared_ptr<Type> ty)
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

		return ty->Name;
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

		// if (interface.WrapperBaseClass == "Wrapper")
		//	GenerateIncludeForWrapper(generator, interface.WrapperClass);

		// ==================
		// == Enumerations ==
		// ==================

		inja::json data;
		data["enums"] = inja::json::array();

		for (const auto& enumeration : interface.Enums)
		{
			if (!enumeration.second.IsOriginalDefinition)
				continue;
			inja::json enumerationData;
			enumerationData["name"]	  = enumeration.first;
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

		//TODO: ExtendedAttributes CustomCppClass=<classname> to wrap external, non-Acorn classes (i.e. glm)

		generator.Add("acorn_root", "Acorn/scripting/v8");
		generator.Add("idl_filename", std::filesystem::path(interface.ModuleOwnPath).filename().string());

		generator.Add("name", interface.Name);
		generator.Add("wrapper_name", std::string(interface.Name + "Wrapper"));
		inja::json data;
		data["constructors"] = inja::json::array();

		for (const auto& ctor : interface.Constructors)
		{
			inja::json ctorData;
			ctorData["cpp_args"] = inja::json::array();
			for (const auto& arg : ctor.Parameters)
			{
				ctorData["cpp_args"].push_back(ToCppType(arg.Type));
			}
			data["constructors"].push_back(ctorData);
		}

		if(interface.WrapperBaseClass != "Wrapper")
		{
			data["inherit"] = interface.WrapperBaseClass;
		} else {
			data["inherit"] = false;
		}

		data["properties"] = inja::json::array();

		for (const auto& prop : interface.Attributes)
		{
			std::cout << prop.Name << " " << prop.GetterNameCallback << ", " << prop.SetterNameCallback << std::endl;
			inja::json propData;
			propData["name"]	= prop.Name;
			propData["readonly"] = prop.ReadOnly ? "true" : "false";
			// TODO GetterNameCallback ExtendedAttributes,....
			propData["virtual"] = prop.ReadOnly;
			data["properties"].push_back(propData);
		}

		data["methods"] = inja::json::array();

		for(const auto& method: interface.Functions)
		{
			if(method.Name.empty())
				continue;
			inja::json methodData;
			methodData["name"] = method.Name;
			//TODO
			data["methods"].push_back(methodData);
		}

		if(interface.IndexedPropertyGetter)
		{
			// TODO do we care about the function definition?
			//FIXME We should definitely assert that the function is a valid indexed getter...
			data["indexed_property_getter"] = true;
		} else {
			data["indexed_property_getter"] = false;
		}

		if(interface.IndexedPropertySetter)
		{
			//FIXME We should definitely assert that the function is a valid indexed setter...
			data["indexed_property_setter"] = {
				{"type", ToCppType(interface.IndexedPropertyGetter->Parameters[0].Type)}
			};
		} else {
			data["indexed_property_setter"] = false;
		}

		if(interface.NamedPropertyGetter)
		{
			// TODO do we care about the function definition?
			//FIXME We should definitely assert that the function is a valid named getter...
			data["named_property_getter"] = true;
		} else {
			data["named_property_getter"] = false;
		}

		if(interface.NamedPropertySetter)
		{
			// TODO do we care about the function definition?
			//FIXME We should definitely assert that the function is a valid named setter...
			data["named_property_setter"] = {
				{"type", ToCppType(interface.IndexedPropertyGetter->Parameters[1].Type)}
			};
		} else {
			data["named_property_setter"] = false;
		}

		if(interface.HasStringifier)
		{
		}

		// FIXME: for now setter without getter does not work,
		// TODO: Deleter

		generator.Add(data);

		std::cout << data << std::endl;


		// generator.Append(WrapperImplementation);
		generator.AppendFile("WrapperImplementation.tpl");

		std::ofstream outFile(outPath.string());
		outFile << generator.Generate();
	}

} // namespace Acorn::IDL
