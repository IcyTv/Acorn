
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "core/Core.h"
#include "core/Log.h"
#include "debug/Instrumentor.h"
#include "utils/FileUtils.h"

#include <corecrt.h>
#include <corecrt_wstdio.h>
#include <fmt/format.h>
#include <fstream>
#include <stdint.h>
#include <unordered_map>
#include <v8.h>
#include <v8pp/convert.hpp>

#include <filesystem>
#include <vector>

constexpr const char* MODULE_CACHE_PATH = "res/cache/scripts";

namespace Acorn
{
	enum class ModuleType
	{
		CommonJS,
		ES6
	};
	struct ModuleData
	{
		std::filesystem::path CachePath;
		ModuleType Type;
	};

	class V8Import
	{
	public:
		// TODO cache compiled module results...
		// TODO cache isolate for reuse with multiple scripts?
		// TODO get the original path of the compiled module to be able to resolve top-level relative imports
		struct CompilerData
		{
			// NOTE maybe just save unsigned char*
			// Map hash to cache file path
			std::unordered_map<std::string, ModuleData> CompileCache;
		};

		static CompilerData s_Data;

		static void Init();
		static void Save();

		static v8::MaybeLocal<v8::Module> LoadModule(const std::string& code, const char* name, v8::Local<v8::Context> cx);
		static v8::Local<v8::Module> CheckModule(v8::MaybeLocal<v8::Module> maybeModule, v8::Local<v8::Context> cx);
		static v8::Local<v8::Value> ExecModule(v8::Local<v8::Module> mod, v8::Local<v8::Context> cx, bool nsObject = false);

		static v8::MaybeLocal<v8::Module> CallResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer);
		static void CallMeta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta);
		static v8::MaybeLocal<v8::Promise> CallDynamic(
			v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions);

		static v8::Local<v8::Module> ResolveBuiltin(v8::Local<v8::Context>, std::string_view specifier, std::string_view nodeModulesPath = Acorn::Utils::File::ResolveResPath("res/scripts/node_modules"));

		static void BindCommonJSRequire(v8::Local<v8::Context> context, v8::Local<v8::Object> global);
		static void CommonJSRequire(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void BindImport(v8::Isolate* isolate);

		static void AddModulePath(int moduleId, const std::filesystem::path& path);

		static std::vector<intptr_t> ExternalRefs()
		{
			return {
				reinterpret_cast<intptr_t>(&V8Import::CommonJSRequire),
			};
		}
	};
}
