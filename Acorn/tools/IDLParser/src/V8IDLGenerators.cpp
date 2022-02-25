/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2021, Luke Wilde <lukew@serenityos.org>
 * Copyright (c) 2022, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "IDLTypes.h"
#include "SourceGenerator.h"

#include <boost/algorithm/string/replace.hpp>

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
		auto wrapperGenerator = generator.Fork();
		wrapperGenerator.Set("wrapper_class", wrapperName);
		// FIXME: These may or may not exist, because REASONS.
		// ^^^^
		// This Fixme is copied straight from Serenity... What the fuck
		// does he mean? What Reasons? What??
		wrapperGenerator.Append(R"~~~(
#if __has_include("@acorn_root@/Bindings/@wrapper_class@.h")
	#include "@acorn_root@/Bindings/@wrapper_class@.h"
#endif
#if __has_include("@acorn_root@/Bindings/@wrapper_class@Factory.h")
	#include "@acorn_root@/Bindings/@wrapper_class@Factory.h"
#endif
		)~~~");
	}

	static void GenerateIncludeForIterator(SourceGenerator& generator, const std::string& iteratorPath, const std::string& iteratorName)
	{
		auto iteratorGenerator = generator.Fork();
		iteratorGenerator.Set("iterator_class.path", iteratorPath);
		iteratorGenerator.Set("iterator_class.name", iteratorName);

		// FIXME see above

		iteratorGenerator.Append(R"~~~(
#include "@acorn_root@/@iterator_class.path@.h"
#if __has_include("@acorn_root@/@iterator_class.path@Factory.h")
	#include "@acorn_root@/@iterator_class.path@Factory.h"
#endif
#if __has_include("@acorn_root@/Bindings/@iterator_class.name@Wrapper.h")
	#include "@acorn_root@/Bindings/@iterator_class.name@Wrapper.h"
#endif
#if __has_include("@acorn_root@/Bindings/@iterator_class.name@WrapperFactory.h")
	#include "@acorn_root@/Bindings/@iterator_class.name@WrapperFactory.h"
#endif
)~~~");
	}

	static void GenerateIncludeFor(SourceGenerator& generator, const std::string& path)
	{
		auto includeGenerator = generator.Fork();
		auto pathStr		  = path;
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
		includeGenerator.Set("include.path", fspath.parent_path().string() + "/" + fspath.filename().string() + ".h");
		includeGenerator.Append(R"~~~(
#include <@include.path@>
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
				auto iterName		 = interface->Name + "Iterator";
				std::string iterPath = interface->FullyQualifiedName + "Iterator";
				boost::replace_all(iterPath, "::", "/");
				GenerateIncludeForIterator(generator, iterPath, iterName);
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
		auto scopedGenerator = generator.Fork();
		auto acceptableName	 = MakeInputAcceptableCpp(cppName);
		scopedGenerator.Set("cpp_name", acceptableName);
		scopedGenerator.Set("js_name", jsName);
		scopedGenerator.Set("js_suffix", jsSuffix);
		scopedGenerator.Set("legacy_null_to_empty_string", legacyNullToEmptyString ? "true" : "false");
		scopedGenerator.Set("parameter.type.name", parameter.Type->Name);
		scopedGenerator.Set("wrapper_name", parameter.Type->Name + "Wrapper");

		if (optionalDefault.has_value())
			scopedGenerator.Set("parameter.optional_default_value", optionalDefault.value());

		// FIXME: Add support for optional, variadic, nullable and
		// default values to all types
		if (parameter.Type->IsString())
		{
			if (variadic)
			{
				scopedGenerator.Append(R"~~~(
			std::vector<std::string> @cpp_name@;
			@cpp_name@.reserve(vm.ArgumentCount() - @js_suffix@);

			for (size_t i = @js_suffix@; i < args.Length(); i++)
			{
				auto toStringResult = args[0].ToString();
				if (toStringResult.IsException())
				{
					return v8::ThrowException(v8::String::New("Could not convert argument to string"));
				}
				@cpp_name@.emplace_back(move(toStringResult.Value()));
			}
				)~~~");
			}
			else if (!optional)
			{
				if (!parameter.Type->Nullable)
				{
					scopedGenerator.Append(R"~~~(
			std::string @cpp_name@;
			if (@js_name@@js_suffix@.IsNull()) {
				@cpp_name@ = "";
			} else {
				@cpp_name@ = @js_name@@js_suffix@.ToString().ToLocalChecked();
			}
					)~~~");
				}
				else
				{
					scopedGenerator.Append(R"~~~(
			std::string @cpp_name@;
			if (!@js_name@@js_suffix@.IsNull()) {
				@cpp_name@ = @js_name@@js_suffix@.ToString().ToLocalChecked();
					)~~~");
				}
			}
			else
			{
				scopedGenerator.Append(R"~~~(
			std::string @cpp_name@;
			if (!@js_name@@js_suffix@.IsUndefined()) {
				if(@js_name@@js_suffix@.IsNull() && @legacy_null_to_empty_string@) {
					@cpp_name@ = "";
				} else {
					@cpp_name@ = @js_name@@js_suffix@.ToString().ToLocalChecked();
				}
			})~~~");

				if (optionalDefault.has_value() && (!parameter.Type->Nullable || optionalDefault.value() != "null"))
				{
					scopedGenerator.Append(R"~~~(
			else {
				@cpp_name@ = @optional_default_value@;
			}
					)~~~");
				}
				else
				{
					scopedGenerator.Append(R"~~~(
					)~~~");
				}
			}
		}
		else if (parameter.Type->Name == "EventListener")
		{
			// FIXME Relace this with support for CallbackInterfaces. https://heycam.github.io/webidl/#idl-callback-interface
			if (parameter.Type->Nullable)
			{
			}
		}
	}

	void GenerateHeader(const Interface& interface)
	{
		std::stringstream builder;
		SourceGenerator generator{ builder };

		generator.Set("acorn_root", "Acorn/scripting/v8");
		generator.Set("idl_filename", interface.ModuleOwnPath);

		generator.Append(R"~~~(
/**
 * This file is generated by @acorn_root@/Generator.cpp from 
 * @idl_filename@.
 *
 * Copyright (c) 2022 Michael Finger
 * 
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 * 
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "@acorn_root@/Wrapper.h"
#include "Acorn/Core.h"
)~~~");

		// for (auto& path : interface->I)
		// {
		// 	GenerateIncludeFor(generator, path);
		// }

		EmitIncludesForAllImports(interface, generator, true);
		generator.Set("name", interface.Name);
		generator.Set("fully_qualified_name", interface.FullyQualifiedName);
		generator.Set("wrapper_class", interface.WrapperClass);
		generator.Set("wrapper_base_class", interface.WrapperBaseClass);

		if (interface.WrapperBaseClass == "Wrapper")
			GenerateIncludeForWrapper(generator, interface.WrapperClass);

		generator.Append(R"~~~(
namespace Acorn::Scripting::V8
{
	class @wrapper_class@: public @wrapper_base_class@
	{
	public:

		virtual void Bind(v8::Local<v8::Object> object) override;

		)~~~");

		if (interface.ExtendedAttributes.contains("CustomGet"))
		{
			generator.Append(R"~~~(
		virtual v8::Local<v8::Value> InternalGet(v8::Local<v8::String> property) override;
		)~~~");
		}
		if (interface.ExtendedAttributes.contains("CustomSet"))
		{
			generator.Append(R"~~~(
		virtual void InternalSet(v8::Local<v8::String> property, v8::Local<v8::Value> value) override;
		)~~~");
		}
		if (interface.ExtendedAttributes.contains("CustomHasProperty"))
		{
			generator.Append(R"~~~(
		virtual bool InternalHasProperty(v8::Local<v8::String> property) override;
		)~~~");
		}

		if (interface.WrapperBaseClass == "Wrapper")
		{
			generator.Append(R"~~~(
		@fully_qualified_name@& Impl() { return *m_Impl; }
		const @fully_qualified_name@& Impl() const { return *m_Impl; }
			)~~~");
		}
		else
		{
			generator.Append(R"~~~(
		@fully_qualified_name@& Impl() { return static_cast<@fully_qualified_name@&>(@wrapper_base_class@::impl()); };
		const @fully_qualified_name@& Impl() const { return static_cast<const @fully_qualified_name@&>(@wrapper_base_class@::impl()); };
		)~~~");
		}

		if (interface.WrapperBaseClass == "Wrapper")
		{
			generator.Append(R"~~~(
		@fully_qualified_name@* m_Impl;
			)~~~");
		}
		generator.Append(R"~~~(
	};
		)~~~");

		for (auto& it : interface.Enums)
		{
			if (!it.second.IsOriginalDefinition)
				continue;

			auto enumGenerator = generator.Fork();
			enumGenerator.Set("enum.type.name", it.first);
			enumGenerator.Append(R"~~~(
	enum class @enum.type.name@ {
		)~~~");

			for (auto& entry : it.second.TranslatedCppNames)
			{
				enumGenerator.Set("enum.entry", entry.second);
				enumGenerator.Append(R"~~~(
		@enum.entry@,
				)~~~");
			}

			enumGenerator.Append(R"~~~(
	};

	inline std::string IDLEnumToString(@enum.type.name@ value) {
		switch(value) {
			)~~~");

			for (auto& entry : it.second.TranslatedCppNames)
			{
				enumGenerator.Set("enum.entry", entry.second);
				enumGenerator.Set("enum.string", entry.first);
				enumGenerator.Append(R"~~~(
			case @enum.type.name@::@enum.entry@: return "@enum.string@";
			)~~~");
			}

			enumGenerator.Append(R"~~~(
			default: return "<unknown>";
		};
	}
			)~~~");
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
		std::cout << generator.AsString() << std::endl;
	}

	void GenerateImplementation(const Interface& interface)
	{
		std::stringstream builder;
		SourceGenerator generator{ builder };

		generator.Set("acorn_root", "Acorn/scripting/v8");
		generator.Set("idl_filename", interface.ModuleOwnPath);

		generator.Set("name", interface.Name);
		generator.Set("wrapper_class", interface.WrapperClass);
		generator.Set("wrapper_base_class", interface.WrapperBaseClass);
		generator.Set("prototype_class", interface.PrototypeClass);
		generator.Set("fully_qualified_name", interface.FullyQualifiedName);

		generator.Append(R"~~~(
/**
 * 
 * This file is generated by @acorn_root@/Generator.cpp from 
 * @idl_filename@.
 *
 * Copyright (c) 2022 Michael Finger
 * 
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 * 
 * For more Information on the license, see the LICENSE.md file
 *
 * Copyright (c) 2022 Michael Finger
 */

#pragma once

#include "Acorn/Core.h"

#include "@acorn_root@/@prototype_class@.h"
#include "@acorn_root@/@wrapper_class@.h"

#include <string_view>
#include <string>
		)~~~");

		EmitIncludesForAllImports(interface, generator, false);

		generator.Append(R"~~~(
namespace Acorn::Scripting::V8
{
	@wrapper_class@* @wrapper_class@::create(v8::Local<v8::Object> global, @fully_qualified_name@& impl)
	{

	}
		)~~~");

		std::cout << "===========\nImplementation:\n========" << std::endl;
		std::cout << generator.AsString() << std::endl;
	}

	void GeneratePrototypeHeader(Interface const&) {}
	void GeneratePrototypeImplementation(Interface const&) {}
	void GenerateIteratorPrototypeHeader(Interface const&) {}
	void GenerateIteratorPrototypeImplementation(Interface const&) {}
	void GenerateIteratorHeader(Interface const&) {}
	void GenerateIteratorImplementation(Interface const&) {}

} // namespace Acorn::IDL
