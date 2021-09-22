#include "acpch.h"

#include "TSCompiler.h"

#include "V8Script.h"
#include "core/Core.h"
#include "core/Log.h"
#include "debug/Instrumentor.h"
#include "utils/FileUtils.h"
#include "utils/v8/V8Import.h"

#include "v8pp/convert.hpp"
#include "v8pp/module.hpp"
#include "v8pp/object.hpp"

#include <magic_enum.hpp>
#include <utility>
#include <v8.h>
#include <v8pp/context.hpp>

//Based on https://gist.github.com/surusek/4c05e4dcac6b82d18a1a28e6742fc23e

namespace Acorn
{

	//TODO v8 debugging?
	// static v8::Handle<v8::Value> Include(const v8::FunctionCallbackInfo<v8::Value>& args)
	// {
	// 	v8::HandleScope scope(args.GetIsolate());
	// 	for (int i = 0; i < args.Length(); i++)
	// 	{
	// 		if (!args[i]->IsString())
	// 			return args.GetIsolate()->ThrowException(v8::Exception::TypeError(v8pp::to_v8(args.GetIsolate(), "Argument must be a string")));

	// 		std::string filename = v8pp::from_v8<std::string>(args.GetIsolate(), args[i]);

	// 		if (filename.length() > 0)
	// 		{
	// 		}
	// 	}
	// }

	static v8::Local<v8::String> toJson(v8::Local<v8::Value> object, v8::Local<v8::Context> context)
	{
		v8::EscapableHandleScope scope(context->GetIsolate());

		v8::Local<v8::Object> global = context->Global();

		v8::Local<v8::Object> JSON = global->Get(context, v8::String::NewFromUtf8(context->GetIsolate(), "JSON").ToLocalChecked()).ToLocalChecked()->ToObject(context).ToLocalChecked();
		v8::Local<v8::Function> JSON_stringify = v8::Local<v8::Function>::Cast(JSON->Get(context, v8::String::NewFromUtf8(context->GetIsolate(), "stringify").ToLocalChecked()).ToLocalChecked());

		v8::Local<v8::Value> argv[3] = {object, v8::Null(context->GetIsolate()), v8::Integer::New(context->GetIsolate(), 4)};

		return scope.Escape(JSON_stringify->Call(context, JSON, 3, argv).ToLocalChecked())->ToString(context).ToLocalChecked();
	}

	static void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		std::stringstream ss;
		v8::HandleScope handle_scope(args.GetIsolate());
		for (int i = 0; i < args.Length(); i++)
		{
			v8::String::Utf8Value str(args.GetIsolate(), args[i]);
			std::string s(*str);
			ss << s << " ";
		}
		AC_CORE_INFO("[V8]: {0}", ss.str());
	}

	static void ReadFile(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		if (args.Length() != 1)
		{
			args.GetIsolate()->ThrowException(v8::Exception::TypeError(v8pp::to_v8(args.GetIsolate(), "Wrong number of arguments")));
			return;
		}

		v8::String::Utf8Value filename(args.GetIsolate(), args[0]);
		std::filesystem::path filename_str(*filename);
		if (!std::filesystem::exists(filename_str))
		{
			args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
			return;
			// args.GetIsolate()->ThrowException(v8::Exception::Error(v8pp::to_v8(args.GetIsolate(), "File not found")));
			// return;
		}
		std::string contents = Utils::File::ReadFile(filename_str.string());
		v8::Local<v8::String> v8Contents = v8pp::to_v8(args.GetIsolate(), contents);
		args.GetReturnValue().Set(v8Contents);
	}

	static void WriteFile(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		if (args.Length() != 2)
		{
			args.GetIsolate()->ThrowException(v8::Exception::Error(v8pp::to_v8(args.GetIsolate(), "Wrong number of arguments")));
			return;
		}

		v8::String::Utf8Value filename(args.GetIsolate(), args[0]);
		v8::String::Utf8Value contents(args.GetIsolate(), args[1]);

		std::filesystem::path filename_str(*filename);
		if (!std::filesystem::exists(filename_str.parent_path()))
		{
			args.GetIsolate()->ThrowException(v8::Exception::Error(v8pp::to_v8(args.GetIsolate(), "Parent directory does not exist")));
			return;
		}

		Utils::File::WriteFile(filename_str.string(), *contents);
	}

	static void FileExists(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		if (args.Length() != 1)
		{
			args.GetIsolate()->ThrowException(v8::Exception::Error(v8pp::to_v8(args.GetIsolate(), "Wrong number of arguments")));
			return;
		}

		v8::String::Utf8Value filename(args.GetIsolate(), args[0]);
		std::filesystem::path filename_str(*filename);

		v8::Local<v8::Boolean> exists = v8pp::to_v8(args.GetIsolate(), std::filesystem::exists(filename_str));
		args.GetReturnValue().Set(exists);
	}

	static void WaitForPendingPromise(const v8::Local<v8::Promise>& promise, v8::Isolate* isolate, const v8::TryCatch& trycatch)
	{
		AC_PROFILE_FUNCTION();
		v8::HandleScope handle_scope(isolate);
		while (promise->State() == v8::Promise::kPending)
		{
			AC_CORE_INFO("Waiting for compiler");
			isolate->GetEnteredOrMicrotaskContext()->GetMicrotaskQueue()->PerformCheckpoint(isolate);
		}
		if (trycatch.HasCaught())
		{
			v8::Local<v8::Value> trace = trycatch.StackTrace(isolate->GetCurrentContext(), trycatch.Exception()).ToLocalChecked();
			AC_CORE_ERROR("Compiler error: {0}", v8pp::from_v8<std::string>(isolate, trace));
			AC_CORE_ERROR("Compiler failed: {0}", v8pp::from_v8<std::string>(isolate, trycatch.Message()->Get()));
		}
		AC_CORE_ASSERT(promise->State() == v8::Promise::kFulfilled, "Promise failed to be fulfilled");
	}

	TSScriptData TSCompiler::Compile(v8::Isolate* isolate, const std::string& filepath)
	{
		AC_PROFILE_FUNCTION();
		{
			AC_PROFILE_SCOPE("std::filesystem::exists(filepath)");
			AC_CORE_ASSERT(std::filesystem::exists(filepath), "File not found");
		}
		AC_CORE_ASSERT(V8Engine::instance().isRunning(), "V8 Engine is not running");
		// V8Engine::instance().KeepRunning();

		// v8::Isolate::CreateParams create_params;
		// create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		// v8::Isolate* isolate = v8::Isolate::New(create_params);

		AC_CORE_ASSERT(isolate, "Failed to create V8 isolate!");
		AC_CORE_ASSERT(isolate != nullptr, "V8 Isolate is null!");

		TSScriptData data;

		// Create a stack-allocated handle scope.

		{
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
			BindImport(isolate);

			v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
			global->Set(v8pp::to_v8(isolate, "print"), v8::FunctionTemplate::New(isolate, Print));

			v8::Local<v8::ObjectTemplate> fileSystem = v8::ObjectTemplate::New(isolate);
			fileSystem->Set(v8pp::to_v8(isolate, "ReadFile"), v8::FunctionTemplate::New(isolate, ReadFile));
			fileSystem->Set(v8pp::to_v8(isolate, "WriteFile"), v8::FunctionTemplate::New(isolate, WriteFile));
			fileSystem->Set(v8pp::to_v8(isolate, "FileExists"), v8::FunctionTemplate::New(isolate, FileExists));

			fileSystem->Set(v8pp::to_v8(isolate, "CurrentFile"), v8pp::to_v8(isolate, filepath));

			global->Set(v8pp::to_v8(isolate, "AcornFileSystem"), fileSystem);

			v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);
			{
				v8::Context::Scope context_scope(context);
				v8::TryCatch try_catch(isolate);
				std::string source = Utils::File::ReadFile("res/scripts/builtins/compiler.js");
				try
				{
					v8::Local<v8::Module> module = checkModule(loadModule(source, "res/scripts/builtins/compiler.js", context), context);
					AC_CORE_ASSERT(module->GetStatus() == v8::Module::kInstantiated, "Failed to instantiate compiler.js module!");
					//TODO load filepath into script
					v8::Local<v8::Value> ret = execModule(module, context);
					if (try_catch.HasCaught())
						AC_CORE_ERROR("Compiler error: {0}", v8pp::from_v8<std::string>(isolate, try_catch.Message()->Get()));

					if (ret.IsEmpty())
					{
						std::string err = v8pp::from_v8<std::string>(isolate, try_catch.Message()->Get());
						AC_CORE_ASSERT(false, "Compiler error {}", err);
					}

					// AC_CORE_ASSERT(!exports.IsEmpty(), "No commonjs exports");

					AC_CORE_ASSERT(ret->IsPromise(), "Compiler returned non-promise value!");

					v8::Local<v8::Promise> promise = v8::Local<v8::Promise>::Cast(ret);

					if (try_catch.HasCaught())
					{
						std::string err = v8pp::from_v8<std::string>(isolate, try_catch.Message()->Get());
						AC_CORE_ASSERT(false, "Compiler error {}", err);
					}

					WaitForPendingPromise(promise, isolate, try_catch);

					std::string getTypesSrc = Utils::File::ReadFile("res/scripts/builtins/getTypes.js");
					v8::Local<v8::Module> getTypedefsModule = checkModule(loadModule(getTypesSrc, "res/scripts/builtins/getTypes.js", context), context);
					AC_CORE_ASSERT(getTypedefsModule->GetStatus() == v8::Module::kInstantiated, "Failed to instantiate getTypes.js module!");
					v8::Local<v8::Value> ret2 = execModule(getTypedefsModule, context);

					if (ret2.IsEmpty())
					{
						std::string err = v8pp::from_v8<std::string>(isolate, try_catch.Message()->Get());
						AC_CORE_ASSERT(false, "Compiler error {}", err);
					}

					AC_CORE_ASSERT(ret2->IsPromise(), "Compiler returned non-promise value!");

					v8::Local<v8::Promise> promise2 = v8::Local<v8::Promise>::Cast(ret2);
					WaitForPendingPromise(promise2, isolate, try_catch);

					v8::Local<v8::Value> ns2 = getTypedefsModule->GetModuleNamespace();
					AC_CORE_ASSERT(ns2->IsObject(), "Module namespace is not an object!");
					v8::Local<v8::Object> obj2 = ns2->ToObject(context).ToLocalChecked();

					v8::Local<v8::Object> typedefs = obj2->Get(context, v8pp::to_v8(isolate, "documentation")).ToLocalChecked().As<v8::Object>();

					AC_CORE_ASSERT(typedefs->IsObject(), "Typedefs is not an object!");

					v8::Local<v8::Array> components = typedefs->Get(context, v8pp::to_v8(isolate, "component")).ToLocalChecked().As<v8::Array>();
					AC_CORE_ASSERT(components->IsArray(), "Typedefs Component is not an array!");
					AC_CORE_ASSERT(components->Length() == 1, "Typedefs Component is not an array of length 1!");

					v8::Local<v8::Object> classComponent = components->Get(context, 0).ToLocalChecked().As<v8::Object>();

					AC_CORE_ASSERT(classComponent->IsObject(), "Typedefs Component is not an object!");

					v8::Local<v8::String> name = classComponent->Get(context, v8pp::to_v8(isolate, "name")).ToLocalChecked().As<v8::String>();
					data.ClassName = v8pp::from_v8<std::string>(isolate, name);

					v8::Local<v8::Array> fields = typedefs->Get(context, v8pp::to_v8(isolate, "fields")).ToLocalChecked().As<v8::Array>();
					AC_CORE_ASSERT(fields->IsArray(), "Typedefs fields is not an array!");

					data.Fields.reserve(fields->Length());

					for (uint32_t i = 0; i < fields->Length(); i++)
					{
						TSField field;

						v8::Local<v8::Object> fieldObj = fields->Get(context, i).ToLocalChecked().As<v8::Object>();
						AC_CORE_ASSERT(fieldObj->IsObject(), "Typedefs field is not an object!");

						v8::Local<v8::String> name = fieldObj->Get(context, v8pp::to_v8(isolate, "name")).ToLocalChecked().As<v8::String>();
						field.Name = v8pp::from_v8<std::string>(isolate, name);

						v8::Local<v8::String> type = fieldObj->Get(context, v8pp::to_v8(isolate, "type")).ToLocalChecked().As<v8::String>();
						field.Type = v8pp::from_v8<std::string>(isolate, type);

						v8::Local<v8::String> documentation = fieldObj->Get(context, v8pp::to_v8(isolate, "documentation")).ToLocalChecked().As<v8::String>();
						field.Documentation = v8pp::from_v8<std::string>(isolate, documentation);

						data.Fields.emplace(std::make_pair(field.Name, field));
					}

					// v8::Local<v8::Value> def = typedefs->Get(context, 1).ToLocalChecked();
					// AC_CORE_ASSERT(def->IsObject(), "Typedef is not an object!");
					// AC_CORE_INFO("Typedef: {0}", v8pp::from_v8<std::string>(isolate, toJson(def, context)->ToString(context).ToLocalChecked()));
				}
				catch (std::runtime_error& e)
				{
					if (try_catch.HasCaught())
						AC_CORE_ERROR("{}", v8pp::from_v8<std::string>(isolate, try_catch.Message()->Get()));
					AC_CORE_ASSERT(false, "Compile error: {}", e.what());
				}
			}
		}

		//FIXME[epic=severe] something does not get cleaned up properly and program crashes (platform_ is nullptr)
		// isolate->Dispose();

		// V8Engine::instance().Stop();

		return data;
	}
}