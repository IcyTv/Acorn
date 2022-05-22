
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

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
#include <nlohmann/json.hpp>

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
	std::unordered_map<int, std::filesystem::path> s_ModulePaths;

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

	v8::MaybeLocal<v8::Module> V8Import::LoadModule(const std::string& code, const char* name, v8::Local<v8::Context> cx)
	{
		AC_PROFILE_FUNCTION();
		// AC_CORE_TRACE("Loading {}", name);

		ModuleType type = ModuleType::ES6;

		std::string md5Hash = Utils::File::MD5HashString(code);
		// AC_CORE_TRACE("MD5HashString {} {}", name, md5Hash);

		v8::Local<v8::String> vcode;
		{
			AC_PROFILE_SCOPE("Converting code to v8");
			// vcode = v8pp::to_v8(cx->GetIsolate(), code);
			vcode = v8::String::NewFromUtf8(cx->GetIsolate(), code.c_str(), v8::NewStringType::kInternalized).ToLocalChecked();
		}
		v8::ScriptOrigin origin(cx->GetIsolate(), v8::String::NewFromUtf8(cx->GetIsolate(), name).ToLocalChecked(), 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true);

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

				AC_CORE_ASSERT(!!cacheFile, "Could not open cache file!");
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

	static v8::MaybeLocal<v8::Module> LoadModuleFromPath(const std::filesystem::path& path, v8::Local<v8::Context> context)
	{
		auto fsPath = std::filesystem::weakly_canonical(path);

		AC_CORE_ASSERT(std::filesystem::exists(fsPath), "File {} does not exist!", fsPath.string());
		std::string fileContents		  = Acorn::Utils::File::ReadFile(fsPath.string());
		v8::MaybeLocal<v8::Module> module = V8Import::LoadModule(fileContents, fsPath.string().c_str(), context);
		if(!module.IsEmpty())
		{
			v8::Local<v8::Module> mod = module.ToLocalChecked();
			s_ModulePaths[mod->ScriptId()] = fsPath;
			return module;
		}
		AC_CORE_ERROR("Failed to load module {}", fsPath.string());
		AC_ASSERT_NOT_REACHED();
		return v8::MaybeLocal<v8::Module>();

	}

	v8::MaybeLocal<v8::Module> V8Import::CallResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer)
	{
		AC_PROFILE_FUNCTION();
//		v8::Local<v8::FixedArray> moduleRequests = referrer->GetModuleRequests();
//		v8::Local<v8::ModuleRequest> request = moduleRequests->Get(context, 0).As<v8::ModuleRequest>();
//		v8::Local<v8::String> requestSpecifier = request->GetSpecifier();
//		std::string requestPathStr = v8pp::from_v8<std::string>(context->GetIsolate(), requestSpecifier);
//		std::filesystem::path requestPath = requestPathStr;
//		requestPath = requestPath.parent_path();

		// Get the path of the referrer

		v8::String::Utf8Value str(context->GetIsolate(), specifier);
		std::string specifierStr = *str;

//		std::optional<std::filesystem::path> referrerPath;
//		auto it = s_ModulePaths.find(referrer->ScriptId());
//		if(it != s_ModulePaths.end())
//			referrerPath = it->second;

		// AC_CORE_TRACE("Resolving {}", specifierStr);
		std::filesystem::path path(specifierStr);

		std::filesystem::path resPath = Acorn::Utils::File::ResolveResPath("res/scripts/builtins/");

		// https://nodejs.org/api/modules.html#modules_all_together
		if(specifierStr.starts_with("/"))
		{
			// Absolute path
			// TODO check if is js file...?
			AC_CORE_ASSERT(std::filesystem::exists(path), "File {} does not exist!", specifierStr);

			return LoadModuleFromPath(path, context);
		}
		if(specifierStr.starts_with("./") || specifierStr.starts_with("../"))
		{
			// Relative resolution
			std::filesystem::path referrerPath = s_ModulePaths[referrer->ScriptId()];
			AC_CORE_ASSERT(!referrerPath.empty(), "Could not find referrer path!");
			std::filesystem::path resolvedPath = referrerPath.parent_path() / path;

			if(!resolvedPath.has_extension())
				resolvedPath.replace_extension(".js");

			return LoadModuleFromPath(resolvedPath, context);
		}
		// Include from the node_modules folder in builtins
		std::filesystem::path nodeModulesPath = s_ModulePaths[referrer->ScriptId()].parent_path();
		std::error_code err;
		nodeModulesPath = std::filesystem::canonical(nodeModulesPath, err);
		AC_CORE_ASSERT(!err, "Could not canonicalize node modules path {}!\n{}", nodeModulesPath.string(), err.message());
		// AC_CORE_TRACE("Checking node_modules path {}", nodeModulesPath.string());
		AC_CORE_ASSERT(!nodeModulesPath.empty(), "Could not find referrer path!");
		// It is possible to require specific files or submodules distributed with a module by including a path suffix after the module name.
		// For instance require('example-module/path/to/file') would resolve path/to/file relative to where example-module is located.
		// The suffixed path follows the same module resolution semantics.
		// Node.js will not append node_modules to a path already ending in node_modules.
		while(nodeModulesPath.parent_path().has_parent_path())
		{
			// Node.js starts at the directory of the current module, and adds /node_modules, and attempts to load the module from that location
			nodeModulesPath = nodeModulesPath / "node_modules";
			// AC_CORE_TRACE("Trying to resolve module {}", nodeModulesPath.string());
			if(std::filesystem::exists(nodeModulesPath / specifierStr / "package.json"))
			{
				// Now we have to resolve the module inside the node_modules folder
				std::ifstream packageFile(nodeModulesPath / specifierStr / "package.json");
				AC_CORE_ASSERT(!!packageFile, "Failed to open package.json file for module {}", specifierStr);
				nlohmann::json packageJson;
				packageFile >> packageJson;
				packageFile.close();
				// First try to get the `module` property, then the `main` property
				// Either of those have to exist, otherwise we can't resolve the module
				// FIXME I don't know if this is actually correct (i.e. if you can have a package.json without a module/main property)
				auto modulePath = packageJson.value("module", packageJson["main"]);
				AC_CORE_ASSERT(modulePath.is_string());
				std::filesystem::path resolvedPath = nodeModulesPath / specifierStr / modulePath.get<std::string>();
				AC_CORE_ASSERT(std::filesystem::exists(resolvedPath), "File {} does not exist!", resolvedPath.string());
				return LoadModuleFromPath(resolvedPath, context);
			}
			// Double parent, because otherwise we would loop by removing node_modules only
			nodeModulesPath = nodeModulesPath.parent_path().parent_path();
		}

		AC_ASSERT_NOT_REACHED();
		return v8::MaybeLocal<v8::Module>();

//		std::vector paths {
//			path,
//			resPath / path,
//			modulePath,
//			resPath / modulePath,
//			resPath / "node_modules" / path,
//			resPath / "node_modules" / modulePath,
//		};

//		if(referrerPath)
//			paths.push_back((*referrerPath).parent_path() / path);

//		AC_CORE_INFO("Resolving {}", path.string());
//
//		for(const auto& p : paths)
//		{
//			if (std::filesystem::exists(p))
//			{
//				auto fsPath = std::filesystem::canonical(p);
//				AC_CORE_INFO("Found {}", fsPath.string());
//				std::string file = Acorn::Utils::File::ReadFile(fsPath.string());
//				v8::MaybeLocal<v8::Module> module = V8Import::loadModule(file, fsPath.string().c_str(), context);
//				if(!module.IsEmpty())
//				{
//					v8::Local<v8::Module> mod = module.ToLocalChecked();
//					s_ModulePaths[mod->ScriptId()] = fsPath;
//					return module;
//				}
//				AC_CORE_ERROR("Failed to load module {}", fsPath.string());
//				return v8::MaybeLocal<v8::Module>();
//			}
//		}
//
//		AC_CORE_ERROR("Could not find module {}", path.string());
//		for(const auto& p: paths)
//		{
//			AC_CORE_WARN("Tried {}", p.string());
//		}
//		AC_ASSERT_NOT_REACHED();
//		return v8::MaybeLocal<v8::Module>();
	}

	v8::Local<v8::Module> V8Import::CheckModule(v8::MaybeLocal<v8::Module> maybeModule, v8::Local<v8::Context> cx)
	{
		AC_PROFILE_FUNCTION();
		v8::HandleScope handle_scope(cx->GetIsolate());
		v8::Local<v8::Module> mod;
		if (!maybeModule.ToLocal(&mod))
		{
			AC_CORE_ERROR("Failed to load module");
			throw std::runtime_error("Failed to load module");
		}

		v8::Maybe<bool> result = mod->InstantiateModule(cx, CallResolve);

		AC_CORE_INFO("IsSourceText {}; IsSynthetic {}", mod->IsSourceTextModule(), mod->IsSyntheticModule());

		if (result.IsNothing())
		{
			AC_CORE_ERROR("Can't instantiate module");
			throw std::runtime_error("Can't instantiate module");
		}
		return mod;
	}

	v8::Local<v8::Value> V8Import::ExecModule(v8::Local<v8::Module> mod, v8::Local<v8::Context> cx, bool nsObject)
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

	void V8Import::CallMeta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta)
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

	v8::MaybeLocal<v8::Promise> V8Import::CallDynamic(
		v8::Local<v8::Context> context,
		v8::Local<v8::ScriptOrModule> referrer,
		v8::Local<v8::String> specifier,
		v8::Local<v8::FixedArray> import_assertions)
	{
		AC_PROFILE_FUNCTION();
		v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
		v8::MaybeLocal<v8::Promise> maybePromise  = resolver->GetPromise();

		v8::String::Utf8Value name(context->GetIsolate(), specifier);
		v8::Local<v8::Module> mod	  = CheckModule(LoadModule(Utils::File::ReadFile(*name), *name, context), context);
		v8::Local<v8::Value> retValue = ExecModule(mod, context, true);

		resolver->Resolve(context, retValue).Check();
		return maybePromise;
	}

	static std::filesystem::path ResolvePath(std::string_view modulePath)
	{
		// FIXME
		AC_PROFILE_FUNCTION();
		std::filesystem::path path(modulePath);
		if (std::filesystem::exists(path))
		{
			return path;
		}
		else
		{
			path = Acorn::Utils::File::ResolveResPath("res/scripts/builtins/") / path;
			if (std::filesystem::exists(path))
			{
				return path;
			}
			else
			{
				AC_CORE_ERROR("Failed to resolve path {}", modulePath);
				throw std::runtime_error("Failed to resolve path");
			}
		}
	}

	// http://wiki.commonjs.org/wiki/Modules/1.1.1
	void V8Import::CommonJSRequire(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		// FIXME implement to spec
		AC_PROFILE_FUNCTION();

		v8::Isolate* isolate	   = args.GetIsolate();
		v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

		v8::Local<v8::String> modulePath;
		if (args.Length() == 1 && args[0]->IsString())
		{
			modulePath = args[0]->ToString(ctx).ToLocalChecked();
		}
		else
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Require takes a string argument").ToLocalChecked()));
		}

		std::filesystem::path path = ResolvePath(v8pp::from_v8<std::string>(ctx->GetIsolate(), modulePath));

		v8::ScriptOrigin origin(modulePath);
		v8::ScriptCompiler::Source source(v8pp::to_v8(ctx->GetIsolate(), Utils::File::ReadFile(path.string())), origin);
		v8::Local<v8::Script> script  = v8::ScriptCompiler::Compile(ctx, &source).ToLocalChecked();
		v8::Local<v8::Value> retValue = script->Run(ctx).ToLocalChecked();

		// FIXME replace asserts with isolate errors?
		AC_CORE_ASSERT(!retValue.IsEmpty(), "Failed to load module");

		v8::MaybeLocal<v8::Value> maybeModule = ctx->Global()->Get(ctx, v8::String::NewFromUtf8(ctx->GetIsolate(), "module").ToLocalChecked());
		v8::Local<v8::Value> module;
		if (!maybeModule.ToLocal(&module))
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Failed to get module object for CommonJS module").ToLocalChecked()));
		}

		AC_CORE_ASSERT(module->IsObject(), "'module' is not an object");

		v8::Local<v8::Object> moduleObject = module->ToObject(ctx).ToLocalChecked();

		v8::MaybeLocal<v8::Value> maybeExports = moduleObject->Get(ctx, v8::String::NewFromUtf8(ctx->GetIsolate(), "exports").ToLocalChecked());
		v8::Local<v8::Value> exports;
		if (!maybeExports.ToLocal(&exports))
		{
			// TODO should we actually throw here? or just return undefined? or an empty object?
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Failed to get exports object for CommonJS module").ToLocalChecked()));
		}

		AC_CORE_ASSERT(exports->IsObject(), "'exports' is not an object");

		v8::Local<v8::Object> exportsObject = exports->ToObject(ctx).ToLocalChecked();

		args.GetReturnValue().Set(exportsObject);
	}

	void V8Import::BindCommonJSRequire(v8::Local<v8::Context> context, v8::Local<v8::Object> global)
	{
		AC_PROFILE_FUNCTION();
		v8::Isolate* isolate					= context->GetIsolate();
		v8::Local<v8::String> requireName		= v8::String::NewFromUtf8(isolate, "require").ToLocalChecked();
		v8::Local<v8::Function> requireFunction = v8::Function::New(context, CommonJSRequire, v8::Local<v8::Value>(), 0, v8::ConstructorBehavior::kThrow).ToLocalChecked();
		global->Set(context, requireName, requireFunction).Check();

		v8::Local<v8::String> moduleName   = v8::String::NewFromUtf8(isolate, "module").ToLocalChecked();
		v8::Local<v8::Object> moduleObject = v8::Object::New(isolate);
		moduleObject->Set(context, v8::String::NewFromUtf8(isolate, "exports").ToLocalChecked(), v8::Object::New(isolate)).Check();
		global->Set(context, moduleName, moduleObject).Check();
	}

	void V8Import::BindImport(v8::Isolate* isolate)
	{
		AC_PROFILE_FUNCTION();
		// TODO use synthetic module to bind module.export?

		//  Binding dynamic import() callback
		isolate->SetHostImportModuleDynamicallyCallback(CallDynamic);

		// Binding metadata loader callback
		isolate->SetHostInitializeImportMetaObjectCallback(CallMeta);

		// Bind CommonJS require
		BindCommonJSRequire(isolate->GetCurrentContext(), isolate->GetCurrentContext()->Global());
	}

	void V8Import::AddModulePath(int moduleId, const std::filesystem::path& path)
	{
		s_ModulePaths[moduleId] = path;
	}

	v8::Local<v8::Module> V8Import::ResolveBuiltin(v8::Local<v8::Context> context, std::string_view specifier, std::string_view nodeModulesPath)
	{
		AC_PROFILE_FUNCTION();
		std::filesystem::path path(nodeModulesPath);
		path /= specifier;
		v8::MaybeLocal<v8::Module> mod = LoadModuleFromPath(path, context);
		AC_CORE_ASSERT(!mod.IsEmpty(), "Failed to load builtin module {} from: {}", specifier, path.string());
		return mod.ToLocalChecked();
	}
}