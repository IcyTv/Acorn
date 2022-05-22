
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#include "acpch.h"

#include "ecs/components/V8Script.h"

#include "core/Application.h"
#include "debug/Timer.h"
#include "ecs/components/Components.h"
#include "ecs/components/ScriptableEntity.h"
#include "ecs/components/TSCompiler.h"
#include "input/Input.h"
#include "input/KeyCodes.h"
#include "physics/Collider.h"
#include "utils/FileUtils.h"
#include "utils/v8/V8Import.h"
#include "v8pp/ptr_traits.hpp"

#include <boost/variant/detail/apply_visitor_delayed.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <corecrt_wstdio.h>
#include <fstream>
#include <functional>
#include <glm/detail/qualifier.hpp>
#include <memory>
#include <sstream>

#include <magic_enum.hpp>

#include <filesystem>
#include <libplatform/libplatform.h>
#include <stdexcept>
#include <utility>
#include <v8-platform.h>
#include <v8.h>

#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>

#include "ecs/components/V8Script_internals.h"
#include "utils/v8/V8GlmConversions.h"

#include "TransformWrapper.h"
#include "GlobalWrapper.h"

#define COMPONENT_SWITCH_HAS(name) COMPONENT_SWITCH_HAS2(name, name)

#define COMPONENT_SWITCH_HAS2(name, componentName)                                                                                                                                 \
	case ComponentsEnum::name: args.GetReturnValue().Set(obj->HasComponent<Components::componentName>()); break;

template <>
struct v8pp::convert<std::pair<float, float>>
{
	using from_type = std::pair<float, float>;
	using to_type	= v8::Local<v8::Array>;

	static bool is_valid(v8::Isolate*, v8::Local<v8::Value> value)
	{
		return !value.IsEmpty() && value->IsArray() && value.As<v8::Array>()->Length() == 2;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if (!is_valid(isolate, value))
			throw std::invalid_argument("Expected [x, y] array");

		v8::HandleScope handle_scope(isolate);
		v8::Local<v8::Array> arr = value.As<v8::Array>();

		from_type result;
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		result.first				   = v8pp::from_v8<float>(isolate, arr->Get(context, 0).ToLocalChecked());
		result.second				   = v8pp::from_v8<float>(isolate, arr->Get(context, 1).ToLocalChecked());

		return result;
	}

	static to_type to_v8(v8::Isolate* isolate, from_type const& value)
	{
		v8::EscapableHandleScope scope(isolate);

		v8::Local<v8::Array> arr	   = v8::Array::New(isolate, 2);
		v8::Local<v8::Context> context = isolate->GetCurrentContext();
		arr->Set(context, 0, v8pp::to_v8(isolate, value.first)).Check();
		arr->Set(context, 1, v8pp::to_v8(isolate, value.second)).Check();

		return scope.Escape(arr);
	}
};

template <>
struct v8pp::convert<Acorn::KeyCode>
{
	using from_type = Acorn::KeyCode;
	using to_type	= v8::Local<v8::Value>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if (value.IsEmpty())
			return false;
		if (value->IsNumber())
		{
			int enumNumber = v8pp::from_v8<int>(isolate, value);
			return magic_enum::enum_contains<Acorn::KeyCode>(enumNumber);
		}
		else if (value->IsString())
		{
			std::string enumName = v8pp::from_v8<std::string>(isolate, value);
			return magic_enum::enum_contains<Acorn::KeyCode>(enumName);
		}
		return false;
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if (!is_valid(isolate, value))
			throw std::invalid_argument("Expected valid KeyCode (as an int or string)");

		v8::HandleScope handle_scope(isolate);
		if (value->IsNumber())
		{
			int enumNumber = v8pp::from_v8<int>(isolate, value);
			return magic_enum::enum_value<Acorn::KeyCode>(enumNumber);
		}
		else
		{
			// Value must be a string as verivied in is_valid
			std::string enumName = v8pp::from_v8<std::string>(isolate, value);
			auto result			 = magic_enum::enum_cast<Acorn::KeyCode>(enumName);
			if (!result.has_value())
				throw std::invalid_argument("Expected valid KeyCode (as an int or string)");
			return result.value();
		}
	}

	static to_type to_v8(v8::Isolate* isolate, from_type const& value)
	{
		v8::EscapableHandleScope scope(isolate);

		v8::Local<v8::Value> enumValue = v8pp::to_v8(isolate, magic_enum::enum_name(value));
		return scope.Escape(enumValue);
	}
};

namespace Acorn
{
	std::unordered_map<std::string, V8Script*> s_Scripts;

	class StringVisitor : public boost::static_visitor<std::string>
	{
public:
		std::string operator()(std::string s) const
		{
			return s;
		}

		std::string operator()(float s) const
		{
			return std::to_string(s);
		}

		std::string operator()(bool s) const
		{
			return s ? "true" : "false";
		}
	};

	V8Types ToV8Type(TsType type)
	{
		// TODO
		// else if (type == "object")
		// 	return V8Types::Object;
		// else if (type == "function")
		// 	return V8Types::Function;
		// else if (type == "undefined")
		// 	return V8Types::Undefined;
		// else if (type == "null")
		// 	return V8Types::Null;
		// else if (type == "array")
		// 	return V8Types::Array;
		switch (type)
		{
		case TsType::Number: return V8Types::Number;
		case TsType::String: return V8Types::String;
		case TsType::Boolean: return V8Types::Boolean;
		case TsType::BigInt: return V8Types::Number;
		default: return V8Types::Unknown;
		}
	}

	using V8Transform = v8pp::class_<Components::Transform>;
	using V8Vec3	  = v8pp::class_<glm::vec3>;
	using namespace Acorn::Scripting::V8;

	// static v8::Handle<v8::Object> transform_ref;

	enum class ComponentsEnum : uint16_t
	{
		Tag				 = 0,
		Transform		 = 1,
		SpriteRenderer	 = 2,
		Camera			 = 3,
		NativeScript	 = 4,
		V8Script		 = 5,
		RigidBody2d		 = 6,
		BoxCollider2d	 = 7,
		CircleCollider2d = 8,
	};

	//===============================================================================================//
	//											V8Engine											 //
	//===============================================================================================//

	V8Engine::V8Engine() {}

	V8Engine::~V8Engine()
	{
		Shutdown();
	}

	void V8Engine::Initialize()
	{
		AC_PROFILE_FUNCTION();
		V8Import::Init();
		// std::string const v8_flags = "--turbo_instruction_scheduling --native-code-counters --expose_gc --print_builtin_code --print_code_verbose --profile_deserialization
		// --serialization_statistics --random-seed 314159265"; v8::V8::SetFlagsFromString(v8_flags.data(), (int)v8_flags.length());
		v8::V8::SetFlagsFromString("--stack_trace_on_illegal --abort_on_uncaught_exception");

		ApplicationCommandLineArgs args = Application::Get().GetCommandLineArgs();
		v8::V8::InitializeICUDefaultLocation(args.Args[0]);
		v8::V8::InitializeExternalStartupData(args.Args[0]);
		m_Platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(m_Platform.get());
		v8::V8::Initialize();

		m_Running = true;
	}

	void V8Engine::Shutdown()
	{
		AC_PROFILE_FUNCTION();

		for (auto* it : m_Scripts)
		{
			it->Dispose();
		}
		m_Scripts.clear();

		v8::V8::Dispose();
		v8::V8::ShutdownPlatform();
		m_Running = false;
		m_Platform.reset();
		V8Import::Save();
	}

	//===============================================================================================//
	//                                    Static Functions                                           //
	//===============================================================================================//

	template<class T, void (T::*func)(const v8::FunctionCallbackInfo<v8::Value>&)>
	static void JsFunctionTemplate(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		T* obj = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
		if(obj != nullptr)
			(obj->*func)(args);
		else
			AC_CORE_WARN("JsFunctionTemplate: Object is nullptr");
	}

	static void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		AC_PROFILE_FUNCTION();
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

	static void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch)
	{
		v8::HandleScope handleScope(isolate);
		if(!try_catch->HasCaught())
		{
			AC_CORE_ERROR("Uncaught exception");
			AC_ASSERT_NOT_REACHED();
			return;
		}
		v8::String::Utf8Value exception(isolate, try_catch->Exception());
		std::string exceptionString(*exception);
		v8::Local<v8::Message> message = try_catch->Message();
		if (message.IsEmpty())
		{
			// V8 didn't provide any extra information about this error; just
			// print the exception.
			AC_CORE_ERROR("{0}", exceptionString);
		}
		else
		{
			// Print (filename):(line number): (message).
			v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
			v8::Local<v8::Context> context(isolate->GetCurrentContext());
			std::string filenameString(*filename);
			int lineNumber = message->GetLineNumber(context).FromJust();
			AC_CORE_ERROR("{0}:{1}:{2}", filenameString, lineNumber, exceptionString);
			// Print line of source code.
			v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(context).ToLocalChecked());
			std::string sourcelineString(*sourceline);
			AC_CORE_ERROR("{0}", sourcelineString);
			// Print wavy underline (GetUnderline is deprecated).
			int start = message->GetStartColumn(context).FromJust();
			int end	  = message->GetEndColumn(context).FromJust();
			AC_CORE_ERROR("{: >{}}{:^>{}}", "", start, "^", end - start);
			v8::Local<v8::Value> v8StackTraceString;
			if (try_catch->StackTrace(context).ToLocal(&v8StackTraceString) && v8StackTraceString->IsString() && v8::Local<v8::String>::Cast(v8StackTraceString)->Length() > 0)
			{
				v8::String::Utf8Value stack_trace(isolate, v8StackTraceString);
				std::string stackTraceString(*stack_trace);
				AC_CORE_ERROR("{0}", stackTraceString);
			}
		}
		AC_CORE_BREAK();
	}


	//===============================================================================================//
	//											V8Script											 //
	//===============================================================================================//

	// TODO allow empty methods!
	// TODO add support for other Scripting Methods (onDestroy, onKeyDown,...)
	// TODO add support for async methods? -> If return type is async, add it to a queue that gets resolved after/during other scripts?
	V8Script::V8Script(Entity entity)
		: m_Entity(entity)
	{
		// V8Script(std::string("res/scripts/test.ts"));
	}

	V8Script::V8Script(Entity entity, const std::string& filePath) : m_Isolate(NULL), m_Entity(entity)
	{
		AC_CORE_ASSERT(filePath.ends_with(".ts"), "Script must be a typescript file!");
		m_TSFilePath = filePath;
		m_JSFilePath = filePath.substr(0, filePath.length() - 2) + "js";

		s_Scripts.emplace(m_TSFilePath, this);

		m_Data = TSCompiler::Compile(m_TSFilePath);
	}

	V8Script::~V8Script()
	{
		m_Isolate->Dispose();
		m_Isolate = nullptr;
		V8Engine::instance().RemoveScript(this);

		s_Scripts.erase(s_Scripts.find(m_TSFilePath));
	}

	void V8Script::GetComponent(const v8::FunctionCallbackInfo<v8::Value>& args) {
		AC_PROFILE_FUNCTION();
		v8::Isolate* isolate = args.GetIsolate();
		if(args.Length() != 1)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Invalid arguments")));
			return;
		}

		v8::EscapableHandleScope handle_scope(isolate);

		Acorn::Scripting::V8::ComponentTypes type = v8pp::from_v8<Acorn::Scripting::V8::ComponentTypes>(isolate, args[0]);
		AC_CORE_TRACE("GetComponent: {}", magic_enum::enum_name(type));


		void* ptr = nullptr;
		switch (type)
		{
		case Acorn::Scripting::V8::ComponentTypes::Transform:
			args.GetReturnValue().Set(TransformWrapper::Wrap(isolate, &m_Entity.GetComponent<Transform>()));
			break;
		case Acorn::Scripting::V8::ComponentTypes::Tag:
			ptr = (void*)&m_Entity.GetComponent<Acorn::Components::Tag>();
			break;
		default:
			isolate->ThrowException(v8::Exception::ReferenceError(v8pp::to_v8(isolate, "Unknown component type")));
		}
	}

	// TODO ts->js filename interop
	void V8Script::Load(Entity entity)
	{
		if (m_Isolate)
		{
			return;
		}

		AC_PROFILE_FUNCTION();
		V8Engine::instance().AddScript(this);

		v8::Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

		m_Isolate = v8::Isolate::New(create_params);

		std::string sourceCode = Utils::File::ReadFile(m_JSFilePath);

		std::string md5Hash = Utils::File::MD5HashString(sourceCode);

		AC_CORE_TRACE("TSCompilation succeeded");

		AC_CORE_ASSERT(std::filesystem::exists(m_JSFilePath), "Failed to find compiled script!");

		v8::Isolate::Scope isolate_scope(m_Isolate);
		// Block for destroying handle scope before creating startup blob
		{
			// Create a stack-allocated handle scope.
			v8::HandleScope handle_scope(m_Isolate);

			v8::Local<v8::Context> context = CreateShellContext();

			context->Global()->Set(context, v8pp::to_v8(m_Isolate, "global"), context->Global());

			AC_CORE_TRACE("Serialized Context!");

			m_Context = v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>>(m_Isolate, context);

			AC_CORE_ASSERT(!context.IsEmpty(), "Failed to create V8 context!");
			// Enter the context for compiling and running the hello world script.
			v8::Context::Scope context_scope(context);
			{

				v8::TryCatch trycatch(m_Isolate);
				V8Import::BindCommonJSRequire(context, context->Global());
				V8Import::BindImport(m_Isolate);
				// Create a string containing the JavaScript source code.
				v8::Local<v8::String> source = v8::String::NewFromUtf8(m_Isolate, sourceCode.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
				v8::Local<v8::String> resource = v8pp::to_v8(m_Isolate, m_JSFilePath);

				v8::ScriptOrigin origin(resource, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true);

				// Compile the source code.
				v8::ScriptCompiler::Source scriptSource(source, origin);
				v8::Local<v8::Module> script = v8::ScriptCompiler::CompileModule(m_Isolate, &scriptSource).ToLocalChecked();

				if(script->GetStatus() == v8::Module::kErrored)
				{
					auto exception = script->GetException();
					AC_CORE_ERROR("{0}", *v8::String::Utf8Value(m_Isolate, exception));
					ReportException(m_Isolate, &trycatch);
					return;
				}

				V8Import::AddModulePath(script->ScriptId(), m_JSFilePath);

				auto maybeInstantiated = script->InstantiateModule(context, V8Import::CallResolve);
				AC_CORE_ASSERT(maybeInstantiated.IsJust() && maybeInstantiated.ToChecked(), "Failed to instantiate module!");

				if(trycatch.HasCaught())
				{
					ReportException(m_Isolate, &trycatch);
					return;
				}

				v8::MaybeLocal<v8::Value> v = script->Evaluate(context);
				if(script->GetStatus() == v8::Module::kErrored)
				{
					auto exception = script->GetException();
					AC_CORE_ERROR("{0}", *v8::String::Utf8Value(m_Isolate, exception));
					AC_ASSERT_NOT_REACHED();
					return;
				}
				AC_CORE_ASSERT(script->GetStatus() == v8::Module::kEvaluated, "Failed to evaluate module!");
				auto module = v.ToLocalChecked();
				AC_CORE_ASSERT(module->IsPromise(), "Module export is not a promise!");

				auto promise = v8::Local<v8::Promise>::Cast(module);
				AC_CORE_ASSERT(promise->State() == v8::Promise::kFulfilled, "Module export failed, because the promise couldn't be fulfilled!");
				auto maybe = promise->Result();
				if(trycatch.HasCaught())
				{
					ReportException(m_Isolate, &trycatch);
					return;
				}
				AC_CORE_ASSERT(!maybe.IsEmpty(), "Failed to get module export!");

				auto ns = script->GetModuleNamespace();
				AC_CORE_ASSERT(!ns.IsEmpty(), "Failed to get module namespace!");
				AC_CORE_ASSERT(ns->IsObject(), "Module namespace is not an object!");

				auto nsObject = v8::Local<v8::Object>::Cast(ns);

				auto defaultExport = nsObject->Get(context, v8pp::to_v8(m_Isolate, "default")).ToLocalChecked();
				AC_CORE_ASSERT(defaultExport->IsFunction(), "Default export is not a function!");
				v8::Local<v8::Function> classObj = v8::Local<v8::Function>::Cast(defaultExport);

				AC_CORE_ASSERT(classObj->IsConstructor());

				AC_CORE_TRACE("Class Object Name: {}", *v8::String::Utf8Value(m_Isolate, classObj->GetName()));

				m_Name = v8pp::from_v8<std::string>(m_Isolate, classObj->GetDebugName());

				auto obj = classObj->NewInstanceWithSideEffectType(context, 0, nullptr, v8::SideEffectType::kHasSideEffectToReceiver).ToLocalChecked().As<v8::Object>();
				v8::Local<v8::Object> instance = obj.As<v8::Object>();
				AC_CORE_ASSERT(instance->IsObject(), "Failed to create instance!");
				auto protoStr = instance->ObjectProtoToString(context).ToLocalChecked();
				AC_CORE_TRACE("Class Object Prototype: {}", *v8::String::Utf8Value(m_Isolate, protoStr));

				{
//					Acorn::Scripting::V8::ScriptSuperClass* superClass = v8pp::class_<Acorn::Scripting::V8::ScriptSuperClass>::unwrap_object(m_Isolate, instance);
//					AC_CORE_ASSERT(!!superClass, "Every script must inherit from ScriptSuperClass!");
//					superClass->SetEntity(entity);

					// Acorn::Scripting::V8::ScriptSuperClass& superClass = ScriptSuperClassWrapper::Unwrap(m_Isolate, instance);
					// superClass.SetEntity(entity);
				}

				m_Class.Reset(m_Isolate, classObj);

				v8::Local<v8::Object> prototype = instance->GetPrototype().As<v8::Object>();
				AC_CORE_ASSERT(prototype->IsObject(), "Prototype is not an object!");

				auto keys = prototype->GetPropertyNames(context).ToLocalChecked();
				AC_CORE_TRACE("Prototype: ");
				for(uint32_t i = 0; i < keys->Length(); i++)
				{
					auto key = keys->Get(context, i).ToLocalChecked();
					auto name = v8pp::from_v8<std::string>(m_Isolate, key);
					AC_CORE_TRACE("\t{0}", name);
				}

				auto instanceKeys = instance->GetPropertyNames(context).ToLocalChecked();
				AC_CORE_TRACE("Instance: ");
				for(uint32_t i = 0; i < instanceKeys->Length(); i++)
				{
					auto key = instanceKeys->Get(context, i).ToLocalChecked();
					auto name = v8pp::from_v8<std::string>(m_Isolate, key);
					AC_CORE_TRACE("\t{0}", name);
				}

				auto keys2 = instance->GetOwnPropertyNames(context).ToLocalChecked();
				AC_CORE_TRACE("Instance: ");
				for(uint32_t i = 0; i < keys2->Length(); i++)
				{
					auto key = keys2->Get(context, i).ToLocalChecked();
					auto name = v8pp::from_v8<std::string>(m_Isolate, key);
					AC_CORE_TRACE("\t{0}", name);
				}

				// TODO make optional
				v8::Local<v8::Function> onUpdateFunc = instance->Get(context, v8pp::to_v8(m_Isolate, "onUpdate")).ToLocalChecked().As<v8::Function>();
				AC_CORE_ASSERT(onUpdateFunc->IsFunction());
				AC_CORE_TRACE("OnUpdate: {}", v8pp::from_v8<std::string>(m_Isolate, onUpdateFunc->TypeOf(m_Isolate)));
				m_OnUpdate.Reset(m_Isolate, onUpdateFunc);

				v8::Local<v8::Function> onCreateFunc = prototype->Get(context, v8pp::to_v8(m_Isolate, "onCreate")).ToLocalChecked().As<v8::Function>();
				AC_CORE_ASSERT(onCreateFunc->IsFunction(), "Script must implement OnCreate!");
				AC_CORE_TRACE("OnCreate: {}", v8pp::from_v8<std::string>(m_Isolate, onCreateFunc->TypeOf(m_Isolate)));
				auto ret = onCreateFunc->Call(context, instance, 0, nullptr);

				if (ret.IsEmpty() && trycatch.HasCaught())
				{
					ReportException(m_Isolate, &trycatch);
				}
				else if (ret.IsEmpty())
				{
					AC_CORE_ERROR("[V8]: Failed to call OnCreate!");
					AC_CORE_BREAK();
				}

			}
		}
	}

	void V8Script::Dispose()
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_INFO("V8 Isolate {} != {}", (void*) m_Isolate, (void*) NULL);
		// FIXME we can do better
		//  Keep the isolate alive until the end of the program
		m_Isolate->Dispose();
		m_Isolate = nullptr;
		V8Engine::instance().RemoveScript(this);
	}

	void V8Script::Compile()
	{
		AC_PROFILE_FUNCTION();
		m_Data = TSCompiler::Compile(m_TSFilePath);
	}

	void V8Script::Watch()
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_TRACE("Watching {}", m_TSFilePath);
		// filewatch::FileWatch<std::string> watch(
		// 	m_TSFilePath,
		// 	[](const std::string& path, const filewatch::Event changeType)
		// 	{
		// 		AC_PROFILE_FUNCTION();
		// 		if (changeType == filewatch::Event::modified)
		// 		{
		// 			AC_CORE_INFO("File {} changed!", path);
		// 			V8Script* script = s_Scripts[path];
		// 			if (script)
		// 			{
		// 				script->Compile();
		// 			}
		// 			else
		// 			{
		// 				AC_CORE_WARN("Modified Event for unknown script {}!", path);
		// 			}
		// 		}
		// 	});
		AC_ASSERT_NOT_REACHED();
	}

	void V8Script::OnUpdate(Timestep ts, Camera* camera)
	{
		AC_PROFILE_FUNCTION();
		Timer timer;
		AC_CORE_ASSERT(m_Isolate != nullptr, "V8 Isolate is null!");

		v8::Isolate::Scope isolate_scope(m_Isolate);
		{
			v8::HandleScope handle_scope(m_Isolate);

			v8::Local<v8::Context> context = v8::Local<v8::Context>::New(m_Isolate, m_Context);

			v8::Context::Scope context_scope(context);
			{
				AC_CORE_ASSERT(!m_OnUpdate.IsEmpty(), "V8 OnUpdate is null!");
				AC_CORE_ASSERT(!m_Class.IsEmpty(), "V8 Script Instance is null!");
				AC_CORE_ASSERT(!context.IsEmpty(), "V8 Context is null!");

				v8::Local<v8::Value> time = v8::Number::New(m_Isolate, ts.GetSeconds());

				v8::Local<v8::Value> args[1]	 = { time };
				v8::Local<v8::Function> onUpdate = v8::Local<v8::Function>::New(m_Isolate, m_OnUpdate);
				v8::Local<v8::Object> instance	 = v8::Local<v8::Object>::New(m_Isolate, m_Class);
				v8::TryCatch tryCatch(m_Isolate);
				v8::MaybeLocal<v8::Value> res = onUpdate->Call(context, instance, 1, args);

				if (tryCatch.HasCaught())
				{
//					if (!tryCatch.Message().IsEmpty())
//						AC_CORE_ERROR("[V8]: Error on Update: {}", v8pp::from_v8<std::string>(m_Isolate, tryCatch.Message()->Get()));
//					else
//						AC_CORE_ERROR("[V8]: Unknown Error on Update");
					ReportException(m_Isolate, &tryCatch);
				}

				AC_CORE_ASSERT(!res.IsEmpty(), "V8 OnUpdate failed!");
				AC_CORE_ASSERT(res.ToLocalChecked()->IsUndefined(), "V8 OnUpdate returned a value!");

				m_LastExecutionTime = timer.ElapsedMillis();
			}
		}
	}

	v8::Local<v8::Context> V8Script::CreateShellContext()
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_ASSERT(m_Isolate, "Invalid Isolate");
		AC_CORE_ASSERT(!m_Isolate->IsDead(), "Invalid Isolate");

		v8::EscapableHandleScope handleScope(m_Isolate);
		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
		global->Set(v8::String::NewFromUtf8(m_Isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(m_Isolate, Print));

		Acorn::Scripting::V8::GlobalWrapper::Bind(m_Isolate, global);
		Acorn::Scripting::V8::BindComponentTypes(m_Isolate, global);
		Acorn::Scripting::V8::TransformWrapper::Bind(m_Isolate, global);

		auto createFuncTemplate = v8::FunctionTemplate::New(m_Isolate, &JsFunctionTemplate<V8Script, &V8Script::GetComponent>);
		global->Set(v8pp::to_v8(m_Isolate, "GetComponent"), createFuncTemplate);

		global->SetInternalFieldCount(1);

		v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

		context->Global()->SetAlignedPointerInInternalField(0, (void*)this);

		return handleScope.Escape(context);
	}

	template <typename T>
	void V8Script::SetValue(std::string parameterName, T value)
	{
		static_assert(false, "Invalid Type");
	}

	template <>
	void V8Script::SetValue<bool>(std::string parameterName, bool value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter");
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Boolean, "Tried to set invalid parameter");
		// m_Parameters[parameterName] = value;
	}
	template <>

	void V8Script::SetValue<float>(std::string parameterName, float value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter");
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Number, "Tried to set invalid parameter");
		// m_Parameters[parameterName] = value;
	}

	template <>
	void V8Script::SetValue<std::string>(std::string parameterName, std::string value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter");
		AC_CORE_TRACE("Setting {}", value);
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::String, "Tried to set invalid parameter");
		m_Parameters[parameterName] = value;
	}

	template <typename T>
	T& V8Script::GetValue(std::string parameterName)
	{
		static_assert(false, "Invalid Type");
	}

	std::unordered_map<std::string, std::string> V8Script::GetParameters()
	{
		std::unordered_map<std::string, std::string> parameters;
		for (auto& parameter : m_Parameters)
		{
			std::string val				= boost::apply_visitor(StringVisitor(), parameter.second);
			parameters[parameter.first] = val;
		}
		return parameters;
	}

	void V8Script::SetParameters(std::unordered_map<std::string, std::string> params)
	{
		// TODO move ts parsing somewhere, so that we can use it here for type checking!
	}

	template <>
	bool& V8Script::GetValue<bool>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter");
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Boolean, "Tried to get invalid parameter for type boolean");
		if (!m_Parameters.contains(parameterName))
		{
			// TODO parse default value from typescript
			// m_Parameters[parameterName] = false;
		}
		return boost::get<bool>(m_Parameters[parameterName]);
	}

	template <>
	float& V8Script::GetValue<float>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter");
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Number, "Tried to get invalid parameter for type number");
		if (!m_Parameters.contains(parameterName))
		{
			// TODO parse default value from typescript
			// m_Parameters[parameterName] = 0.0f;
		}
		return boost::get<float>(m_Parameters[parameterName]);
	}

	template <>
	std::string& V8Script::GetValue<std::string>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter");
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::String, "Tried to get invalid parameter for type string");
		if (!m_Parameters.contains(parameterName))
		{
			// TODO parse default value from typescript
			m_Parameters[parameterName] = std::string("");
		}
		return boost::get<std::string>(m_Parameters[parameterName]);
	}
} // namespace Acorn