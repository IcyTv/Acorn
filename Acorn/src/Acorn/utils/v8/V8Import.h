#pragma once

#include "core/Core.h"
#include "core/Log.h"
#include "utils/FileUtils.h"

#include <v8.h>
#include <v8pp/convert.hpp>

#include <filesystem>
#include <vector>

namespace Acorn
{
	//TODO cache compiled module results...

	static v8::MaybeLocal<v8::Module> loadModule(const std::string& code, const char* name, v8::Local<v8::Context> cx)
	{
		AC_CORE_TRACE("Loading {}", name);

		v8::Local<v8::String> vcode = v8pp::to_v8(cx->GetIsolate(), code);
		;

		v8::ScriptOrigin origin(
			v8::String::NewFromUtf8(cx->GetIsolate(), name).ToLocalChecked(),
			v8::Integer::New(cx->GetIsolate(), 0),
			v8::Integer::New(cx->GetIsolate(), 0), v8::False(cx->GetIsolate()),
			v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
			v8::False(cx->GetIsolate()), v8::False(cx->GetIsolate()),
			v8::True(cx->GetIsolate()));

		v8::Context::Scope context_scope(cx);
		v8::ScriptCompiler::Source source(vcode, origin);

		// v8::Local<v8::Value> exports = v8::True(cx->GetIsolate());

		// v8::Local<v8::ObjectTemplate> module = v8::ObjectTemplate::New(cx->GetIsolate());
		// module->Set(v8::String::NewFromUtf8(cx->GetIsolate(), "exports").ToLocalChecked(), exports);

		// cx->Global()->Set(cx, v8::String::NewFromUtf8(cx->GetIsolate(), "module").ToLocalChecked(), module->NewInstance(cx).ToLocalChecked()).Check();

		v8::MaybeLocal<v8::Module> mod = v8::ScriptCompiler::CompileModule(cx->GetIsolate(), &source);

		// if (exports.IsEmpty() || exports->IsBoolean())
		// {
		// 	AC_CORE_BREAK();
		// }
		// else if (!exports->IsNullOrUndefined())
		// {
		// 	AC_CORE_TRACE("Exports: {}", v8pp::from_v8<std::string>(cx->GetIsolate(), exports));
		// 	AC_CORE_BREAK();
		// }

		return mod;
	}

	v8::MaybeLocal<v8::Module> callResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer)
	{
		v8::Local<v8::FixedArray> moduleRequests = referrer->GetModuleRequests();
		v8::Local<v8::ModuleRequest> request = moduleRequests->Get(context, 0).As<v8::ModuleRequest>();
		v8::Local<v8::String> requestSpecifier = request->GetSpecifier();
		std::string requestPathStr = v8pp::from_v8<std::string>(context->GetIsolate(), requestSpecifier);
		std::filesystem::path requestPath = requestPathStr;
		requestPath = requestPath.parent_path();

		v8::String::Utf8Value str(context->GetIsolate(), specifier);

		std::filesystem::path path(*str);

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
			//TODO get module path and look for siblings/resolve path from it
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
			//TODO if folder check for index.js
			AC_CORE_TRACE("Tried {}", path.string());
			AC_CORE_TRACE("Tried {}", "res/scripts/builtins/" + path.string());
			AC_CORE_TRACE("Tried {}", requestPath / path);
			AC_CORE_TRACE("Tried {}", "res/scripts/builtins/" + (requestPath / path).string());
			AC_CORE_ASSERT(false, "Failed to load module");
			return v8::MaybeLocal<v8::Module>();
		}
	}

	static v8::Local<v8::Module> checkModule(v8::MaybeLocal<v8::Module> maybeModule, v8::Local<v8::Context> cx)
	{
		v8::Local<v8::Module> mod;
		if (!maybeModule.ToLocal(&mod))
		{
			AC_CORE_ERROR("Failed to load module");
			throw std::runtime_error("Failed to load module");
		}

		v8::Maybe<bool> result = mod->InstantiateModule(cx, callResolve);

		if (result.IsNothing())
		{
			AC_CORE_ERROR("Can't instantiate module");
			throw std::runtime_error("Can't instantiate module");
		}

		//TODO this must somehow run before module instantiation
		// v8::Local<v8::Value> module = cx->Global()->Get(cx, v8::String::NewFromUtf8(cx->GetIsolate(), "module").ToLocalChecked()).ToLocalChecked();
		// //Module should also never be overridden!
		// AC_CORE_ASSERT(module->IsObject(), "Module should be an object");
		// v8::Local<v8::Value> exports = module.As<v8::Object>()->Get(cx, v8::String::NewFromUtf8(cx->GetIsolate(), "exports").ToLocalChecked()).ToLocalChecked();
		// if (!exports->IsNullOrUndefined())
		// {
		// 	v8::Local<v8::Value> ns = mod->GetModuleNamespace();
		// 	AC_CORE_ASSERT(!ns.IsEmpty(), "Module namespace should not be empty");
		// 	AC_CORE_ASSERT(ns->IsObject(), "Module namespace should be an object");
		// 	v8::Local<v8::Object> nsObj = ns.As<v8::Object>();
		// 	v8::Maybe<bool> syntheticDefault = nsObj->Set(cx, v8pp::to_v8(cx->GetIsolate(), "default"), exports);

		// 	if (syntheticDefault.IsNothing())
		// 	{
		// 		AC_CORE_WARN("Could not set default export for commonjs module");
		// 	}
		// 	else
		// 	{
		// 		AC_CORE_INFO("Set default export for commonjs module");
		// 	}

		return mod;
	}

	static v8::Local<v8::Value> execModule(v8::Local<v8::Module> mod, v8::Local<v8::Context> cx, bool nsObject = false)
	{
		v8::Local<v8::Value> retVal;
		if (!mod->Evaluate(cx).ToLocal(&retVal))
		{
			AC_CORE_ERROR("Failed to evaluate module");
			throw std::runtime_error("Failed to evaluate module");
		}

		if (mod->GetStatus() == v8::Module::kErrored)
		{
			AC_CORE_ERROR("Compiler error: {0}", v8pp::from_v8<std::string>(cx->GetIsolate(), mod->GetException()->ToString(cx).ToLocalChecked()));
			AC_CORE_BREAK();
		}
		AC_CORE_ASSERT(mod->GetStatus() == v8::Module::kEvaluated, "Module status should be evaluated");

		if (nsObject)
			return mod->GetModuleNamespace();
		else
			return retVal;
	}

	static void callMeta(v8::Local<v8::Context> context,
						 v8::Local<v8::Module> module,
						 v8::Local<v8::Object> meta)
	{

		// In this example, this is throw-away function. But it shows that you can
		// bind module's url. Here, placeholder is used.
		meta->Set(
				context,
				v8::String::NewFromUtf8(context->GetIsolate(), "url").ToLocalChecked(),
				v8::String::NewFromUtf8(context->GetIsolate(), "https://something.sh")
					.ToLocalChecked())
			.Check();
	}

	static v8::MaybeLocal<v8::Promise> callDynamic(v8::Local<v8::Context> context,
												   v8::Local<v8::ScriptOrModule> referrer,
												   v8::Local<v8::String> specifier)
	{
		v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
		v8::MaybeLocal<v8::Promise> maybePromise = resolver->GetPromise();

		v8::String::Utf8Value name(context->GetIsolate(), specifier);
		v8::Local<v8::Module> mod = checkModule(loadModule(Utils::File::ReadFile(*name), *name, context), context);
		v8::Local<v8::Value> retValue = execModule(mod, context, true);

		resolver->Resolve(context, retValue).Check();
		return maybePromise;
	}

	void BindImport(v8::Isolate* isolate)
	{
		//TODO use synthetic module to bind module.export?
		// Binding dynamic import() callback
		isolate->SetHostImportModuleDynamicallyCallback(callDynamic);

		// Binding metadata loader callback
		isolate->SetHostInitializeImportMetaObjectCallback(callMeta);
	}
}
