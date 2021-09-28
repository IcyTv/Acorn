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
	class V8Import
	{
	public:
		//TODO cache compiled module results...
		//TODO cache isolate for reuse with multiple scripts?
		struct CompilerData
		{
			//NOTE maybe just save unsigned char*
			//Map hash to cache file path
			std::unordered_map<std::string, std::filesystem::path> CompileCache;
		};

		static CompilerData s_Data;

		static void Init();
		static void Save();

		static v8::MaybeLocal<v8::Module> loadModule(const std::string& code, const char* name, v8::Local<v8::Context> cx);
		static v8::Local<v8::Module> checkModule(v8::MaybeLocal<v8::Module> maybeModule, v8::Local<v8::Context> cx);
		static v8::Local<v8::Value> execModule(v8::Local<v8::Module> mod, v8::Local<v8::Context> cx, bool nsObject = false);

		static v8::MaybeLocal<v8::Module> callResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer);
		static void callMeta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta);
		static v8::MaybeLocal<v8::Promise> callDynamic(v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer, v8::Local<v8::String> specifier);

		static void BindImport(v8::Isolate* isolate);
	};
}
