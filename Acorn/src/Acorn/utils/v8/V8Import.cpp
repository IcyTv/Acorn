#include "Tracy.hpp"
#include "acpch.h"

#include "utils/FileUtils.h"
#include "utils/v8/V8Import.h"
#include "v8pp/convert.hpp"

#include <magic_enum.hpp>
#include <unordered_map>
#include <v8.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template <>
	struct convert<std::unordered_map<std::string, Acorn::ModuleData>>
	{
		static Node encode(const std::unordered_map<std::string, Acorn::ModuleData>& rhs)
		{
			Node node;
			for (const auto& [key, value] : rhs)
			{
				node[key] = value;
			}
			return node;
		}

		static bool decode(const Node& node, std::unordered_map<std::string, Acorn::ModuleData>& rhs)
		{
			if (!node.IsMap())
			{
				return false;
			}

			for (auto n : node)
			{
				rhs[n.first.as<std::string>()] = n.second.as<Acorn::ModuleData>();
			}

			return true;
		}
	};

	template <>
	struct convert<Acorn::ModuleData>
	{
		static Node encode(const Acorn::ModuleData& rhs)
		{
			Node node;
			node["CachePath"] = rhs.CachePath.string();
			node["Type"] = magic_enum::enum_name(rhs.Type).data();

			return node;
		}

		static bool decode(const Node& node, Acorn::ModuleData& rhs)
		{
			if (!node.IsMap())
			{
				return false;
			}

			rhs.CachePath = node["CachePath"].as<std::string>();
			auto typeEnum = magic_enum::enum_cast<Acorn::ModuleType>(node["Type"].as<std::string>());
			if (!typeEnum.has_value())
				return false;
			rhs.Type = typeEnum.value();

			return true;
		}
	};
}

namespace Acorn
{

	V8Import::CompilerData V8Import::s_Data;

	void V8Import::Init()
	{
		AC_PROFILE_FUNCTION();
		std::ifstream configFile(Utils::File::CONFIG_FILENAME);

		if (!configFile.is_open())
			return;

		YAML::Node config = YAML::Load(configFile);

		if (config["v8"]["moduleCache"].IsDefined())
		{
			s_Data.CompileCache = config["v8"]["moduleCache"].as<std::unordered_map<std::string, Acorn::ModuleData>>();

			for (const auto& [key, value] : s_Data.CompileCache)
			{
				AC_CORE_INFO("V8: Loaded module cache: {0}", key);
			}
		}
		else
		{
			AC_CORE_WARN("Could not load module cache");
		}
	}

	void V8Import::Save()
	{
		AC_PROFILE_FUNCTION();
		std::ifstream configFile(Utils::File::CONFIG_FILENAME);

		YAML::Node config;
		if (configFile.is_open())
			config = YAML::Load(configFile);
		else
			config = YAML::Load("");

		if (!s_Data.CompileCache.empty())
			config["v8"]["moduleCache"] = s_Data.CompileCache;

		std::ofstream outFile(Utils::File::CONFIG_FILENAME);
		outFile << config;

		outFile.close();
	}

	// V8Import::CompilerData s_Data;

	v8::MaybeLocal<v8::Module> V8Import::loadModule(const std::string& code, const char* name, v8::Local<v8::Context> cx)
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_TRACE("Loading {}", name);

		ModuleType type = ModuleType::ES6;
		// if (code.find("module.exports") != std::string::npos)
		// {
		// 	type = ModuleType::CommonJS;
		// }

		std::string md5Hash = Utils::File::MD5HashString(code);
		AC_CORE_TRACE("MD5HashString {} {}", name, md5Hash);

		v8::Local<v8::String> vcode;
		{
			AC_PROFILE_SCOPE("Converting code to v8");
			// vcode = v8pp::to_v8(cx->GetIsolate(), code);
			vcode = v8::String::NewFromUtf8(cx->GetIsolate(), code.c_str(), v8::NewStringType::kInternalized).ToLocalChecked();
		}
		v8::ScriptOrigin origin(
			v8::String::NewFromUtf8(cx->GetIsolate(), name).ToLocalChecked(),
			v8::Integer::New(cx->GetIsolate(), 0),
			v8::Integer::New(cx->GetIsolate(), 0), v8::False(cx->GetIsolate()),
			v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
			v8::False(cx->GetIsolate()), v8::False(cx->GetIsolate()),
			v8::True(cx->GetIsolate()));

		v8::Context::Scope context_scope(cx);

		// Check cache
		if (s_Data.CompileCache.find(md5Hash) != s_Data.CompileCache.end() && std::filesystem::exists(s_Data.CompileCache[md5Hash].CachePath))
		{
			AC_PROFILE_SCOPE("Cache Hit");

			AC_CORE_ASSERT(s_Data.CompileCache[md5Hash].Type == type, "Cache type mismatch");

			// Load from cache
			std::filesystem::path cachePath = s_Data.CompileCache[md5Hash].CachePath;
			// std::basic_ifstream<uint8_t> cacheFile(cachePath.string(), std::ios::binary);
			uint8_t* cacheData;
			size_t cacheSize;
			// std::vector<uint8_t> cacheData;
			FILE* cacheFile;
			{
				AC_PROFILE_SCOPE("Loading cached file");

				errno_t err;

				// NOTE this is a lot faster at -O0, but maybe ifstream is just as fast at -O3?
				err = fopen_s(&cacheFile, cachePath.string().c_str(), "rb");

				AC_CORE_ASSERT(cacheFile, "Could not open cache file!");
				AC_CORE_ASSERT(err == 0, "Could not open cache file!");

				fseek(cacheFile, 0, SEEK_END);
				cacheSize = ftell(cacheFile);
				fseek(cacheFile, 0, SEEK_SET);

				cacheData = new uint8_t[cacheSize];

				fread(cacheData, 1, cacheSize, cacheFile);

				fclose(cacheFile);
			}

			v8::ScriptCompiler::CachedData* v8cachedData = new v8::ScriptCompiler::CachedData(cacheData, cacheSize);

			v8::ScriptCompiler::Source source(vcode, origin, v8cachedData);

			v8::MaybeLocal<v8::Module> mod;
			if (type == ModuleType::ES6)
			{
				AC_PROFILE_SCOPE("Compiling ES6 Cached Module");
				mod = v8::ScriptCompiler::CompileModule(cx->GetIsolate(), &source, v8::ScriptCompiler::CompileOptions::kConsumeCodeCache);
			}
			else
			{
				AC_PROFILE_SCOPE("Compiling CommonJS Cached Module");
				AC_CORE_ASSERT(false, "CommonJS modules not supported!");
			}

			return mod;
		}
		else
		{
			AC_PROFILE_SCOPE("Cache Miss");
			v8::ScriptCompiler::Source source(vcode, origin);
			// Compile

			v8::MaybeLocal<v8::Module> mod;
			if (type == ModuleType::ES6)
			{
				AC_PROFILE_SCOPE("Compiling ES6 Module");
				mod = v8::ScriptCompiler::CompileModule(cx->GetIsolate(), &source);
			}
			else
			{
				AC_PROFILE_SCOPE("Compiling CommonJS Module");
				AC_CORE_ASSERT(false, "CommonJS modules not supported!");
			}

			v8::ScriptCompiler::CachedData* data;
			if (type == ModuleType::ES6)
			{
				{
					AC_PROFILE_SCOPE("Getting Cache from v8");
					v8::Local<v8::UnboundModuleScript> script = mod.ToLocalChecked()->GetUnboundModuleScript();
					data = v8::ScriptCompiler::CreateCodeCache(script);
				}
				AC_CORE_ASSERT(data != nullptr, "Failed to create code cache");

				{
					AC_PROFILE_SCOPE("Writing cache");
					std::filesystem::path cachePath = MODULE_CACHE_PATH;
					if (!std::filesystem::exists(cachePath))
						std::filesystem::create_directories(cachePath);

					cachePath /= md5Hash;
					cachePath += ".js.cache";

					FILE* cacheFile;
					errno_t err;
					err = fopen_s(&cacheFile, cachePath.string().c_str(), "wb");

					AC_CORE_ASSERT(cacheFile, "Could not open cache file!");
					AC_CORE_ASSERT(err == 0, "Could not open cache file!");

					fwrite(data->data, 1, data->length, cacheFile);

					fclose(cacheFile);

					// std::basic_ofstream<uint8_t> cacheFile(cachePath.string(), std::ios::binary);
					// cacheFile.write(data->data, data->length);
					// cacheFile.close();

					s_Data.CompileCache[md5Hash] = {
						cachePath,
						type};
				}
			}

			return mod;
		}
	}

	v8::MaybeLocal<v8::Module> V8Import::callResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer)
	{
		AC_PROFILE_FUNCTION();
		v8::Local<v8::FixedArray> moduleRequests = referrer->GetModuleRequests();
		v8::Local<v8::ModuleRequest> request = moduleRequests->Get(context, 0).As<v8::ModuleRequest>();
		v8::Local<v8::String> requestSpecifier = request->GetSpecifier();
		std::string requestPathStr = v8pp::from_v8<std::string>(context->GetIsolate(), requestSpecifier);
		std::filesystem::path requestPath = requestPathStr;
		requestPath = requestPath.parent_path();

		v8::String::Utf8Value str(context->GetIsolate(), specifier);

		std::filesystem::path path(*str);

		AC_CORE_INFO("Resolving {}", path.string());

		if (!path.has_extension() || path.extension() != ".js")
		{
			path += ".js";
		}

		if (std::filesystem::exists(path))
		{
			return loadModule(Utils::File::ReadFile(*str), *str, context);
		}
		else if (std::filesystem::exists("res/scripts/builtins/" + path.string()))
		{
			// TODO get module path and look for siblings/resolve path from it
			std::filesystem::path newPath = "res/scripts/builtins/";
			newPath /= path;

			return loadModule(Utils::File::ReadFile(newPath.string()), *str, context);
		}
		else if (std::filesystem::exists(requestPath / path))
		{
			return loadModule(Utils::File::ReadFile((requestPath / path).string()), *str, context);
		}
		else if (std::filesystem::exists("res/scripts/builtins/" + (requestPath / path).string()))
		{
			return loadModule(Utils::File::ReadFile("res/scripts/builtins/" + (requestPath / path).string()), *str, context);
		}
		else
		{
			// TODO if folder check for index.js
			AC_CORE_TRACE("Tried {}", path.string());
			AC_CORE_TRACE("Tried {}", "res/scripts/builtins/" + path.string());
			AC_CORE_TRACE("Tried {}", requestPath / path);
			AC_CORE_TRACE("Tried {}", "res/scripts/builtins/" + (requestPath / path).string());
			AC_CORE_ASSERT(false, "Failed to load module");
			return v8::MaybeLocal<v8::Module>();
		}
	}

	v8::Local<v8::Module> V8Import::checkModule(v8::MaybeLocal<v8::Module> maybeModule, v8::Local<v8::Context> cx)
	{
		AC_PROFILE_FUNCTION();
		v8::HandleScope handle_scope(cx->GetIsolate());
		v8::Local<v8::Module> mod;
		if (!maybeModule.ToLocal(&mod))
		{
			AC_CORE_ERROR("Failed to load module");
			throw std::runtime_error("Failed to load module");
		}

		v8::Maybe<bool> result = mod->InstantiateModule(cx, callResolve);

		AC_CORE_INFO("IsSourceText {}; IsSynthetic {}", mod->IsSourceTextModule(), mod->IsSyntheticModule());

		if (result.IsNothing())
		{
			AC_CORE_ERROR("Can't instantiate module");
			throw std::runtime_error("Can't instantiate module");
		}
		return mod;
	}

	v8::Local<v8::Value> V8Import::execModule(v8::Local<v8::Module> mod, v8::Local<v8::Context> cx, bool nsObject)
	{
		// TODO speed up
		AC_PROFILE_FUNCTION();
		v8::Local<v8::Value> retVal;
		{
			// AC_PROFILE_SCOPE("V8 Module Evaluation");
			ZoneScopedNS("V8 Module Evaluation", 10);
			if (!mod->Evaluate(cx).ToLocal(&retVal))
			{
				AC_CORE_ERROR("Failed to evaluate module");
				throw std::runtime_error("Failed to evaluate module");
			}
		}

		if (mod->GetStatus() == v8::Module::kErrored)
		{
			AC_CORE_ERROR("Compiler error: {0}", v8pp::from_v8<std::string>(cx->GetIsolate(), mod->GetException()->ToString(cx).ToLocalChecked()));
			AC_CORE_BREAK();
		}
		AC_CORE_ASSERT(mod->GetStatus() == v8::Module::kEvaluated, "Module status should be evaluated");

		auto ns = mod->GetModuleNamespace();
		AC_CORE_INFO("Module namespace: {0}", v8pp::from_v8<std::string>(cx->GetIsolate(), ns));

		if (nsObject)
			return mod->GetModuleNamespace();
		else
			return retVal;
	}

	void V8Import::callMeta(v8::Local<v8::Context> context,
							v8::Local<v8::Module> module,
							v8::Local<v8::Object> meta)
	{
		AC_PROFILE_FUNCTION();

		// In this example, this is throw-away function. But it shows that you can
		// bind module's url. Here, placeholder is used.
		meta->Set(
				context,
				v8::String::NewFromUtf8(context->GetIsolate(), "url").ToLocalChecked(),
				v8::String::NewFromUtf8(context->GetIsolate(), "https://something.sh")
					.ToLocalChecked())
			.Check();
	}

	v8::MaybeLocal<v8::Promise> V8Import::callDynamic(
		v8::Local<v8::Context> context,
		v8::Local<v8::ScriptOrModule> referrer,
		v8::Local<v8::String> specifier,
		v8::Local<v8::FixedArray> import_assertions)
	{
		AC_PROFILE_FUNCTION();
		v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
		v8::MaybeLocal<v8::Promise> maybePromise = resolver->GetPromise();

		v8::String::Utf8Value name(context->GetIsolate(), specifier);
		v8::Local<v8::Module> mod = checkModule(loadModule(Utils::File::ReadFile(*name), *name, context), context);
		v8::Local<v8::Value> retValue = execModule(mod, context, true);

		resolver->Resolve(context, retValue).Check();
		return maybePromise;
	}

	void V8Import::BindImport(v8::Isolate* isolate)
	{
		AC_PROFILE_FUNCTION();
		// TODO use synthetic module to bind module.export?
		//  Binding dynamic import() callback
		isolate->SetHostImportModuleDynamicallyCallback(callDynamic);

		// Binding metadata loader callback
		isolate->SetHostInitializeImportMetaObjectCallback(callMeta);
	}
}