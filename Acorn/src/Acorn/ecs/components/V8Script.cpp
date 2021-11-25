#include "acpch.h"

#include "V8Script.h"

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
// #include <v8pp/context.hpp>
#include <v8pp/convert.hpp>
#include <v8pp/module.hpp>
#include <v8pp/object.hpp>

#include <FileWatch.hpp>

#define COMPONENT_SWITCH_HAS(name) COMPONENT_SWITCH_HAS2(name, name)

#define COMPONENT_SWITCH_HAS2(name, componentName)                                 \
	case ComponentsEnum::name:                                                     \
		args.GetReturnValue().Set(obj->HasComponent<Components::componentName>()); \
		break;

template <>
struct v8pp::convert<std::pair<float, float>>
{
	using from_type = std::pair<float, float>;
	using to_type = v8::Local<v8::Array>;

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
		result.first = v8pp::from_v8<float>(isolate, arr->Get(context, 0).ToLocalChecked());
		result.second = v8pp::from_v8<float>(isolate, arr->Get(context, 1).ToLocalChecked());

		return result;
	}

	static to_type to_v8(v8::Isolate* isolate, from_type const& value)
	{
		v8::EscapableHandleScope scope(isolate);

		v8::Local<v8::Array> arr = v8::Array::New(isolate, 2);
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
	using to_type = v8::Local<v8::Value>;

	static bool is_valid(v8::Isolate* isolate, v8::Local<v8::Value> value)
	{
		if (value.IsEmpty())
			return false;
		if (value->IsNumber())
		{
			//Attention: We cannot envorce integers, so we just assume
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
			//Value must be a string as verivied in is_valid
			std::string enumName = v8pp::from_v8<std::string>(isolate, value);
			auto result = magic_enum::enum_cast<Acorn::KeyCode>(enumName);
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
		if (type == TsType::Number)
			return V8Types::Number;
		else if (type == TsType::String)
			return V8Types::String;
		else if (type == TsType::Boolean)
			return V8Types::Boolean;
		else if (type == TsType::BigInt)
			return V8Types::Number; //TODO
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
		else
			return V8Types::Unknown;
	}

	using V8Transform = v8pp::class_<Components::Transform>;
	using V8Vec3 = v8pp::class_<glm::vec3>;

	// static v8::Handle<v8::Object> transform_ref;

	enum class ComponentsEnum : uint16_t
	{
		Tag = 0,
		Transform = 1,
		SpriteRenderer = 2,
		Camera = 3,
		NativeScript = 4,
		V8Script = 5,
		RigidBody2d = 6,
		BoxCollider2d = 7,
		CircleCollider2d = 8,
	};

	class ScriptSuperClass : public ScriptableEntity
	{
	public:
		~ScriptSuperClass()
		{
		}
	};

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

	static void ScriptSuperClassConstructor(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		AC_PROFILE_FUNCTION();
		v8::Isolate* isolate = args.GetIsolate();
		if (!args.IsConstructCall())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Cannot call constructor as function")));
			return;
		}

		ScriptSuperClass* obj = new ScriptSuperClass();

		args.This()->SetInternalField(0, v8::External::New(isolate, obj));
	}

	static void ScriptSuperClassHasComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		AC_PROFILE_FUNCTION();
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope handle_scope(isolate);

		if (args.Length() != 1)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Wrong number of arguments")));
			return;
		}

		if (!args[0]->IsUint32())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Only Accepts a number")));
			return;
		}

		uint16_t component = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
		auto componentEnum = magic_enum::enum_cast<ComponentsEnum>(component);
		if (!componentEnum.has_value())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Invalid Component")));
			return;
		}

		ScriptSuperClass* obj = static_cast<ScriptSuperClass*>(v8::Local<v8::External>::Cast(args.This()->GetInternalField(0))->Value());
		switch (componentEnum.value())
		{
			COMPONENT_SWITCH_HAS(Tag)
			COMPONENT_SWITCH_HAS(Transform)
			COMPONENT_SWITCH_HAS(SpriteRenderer)
			COMPONENT_SWITCH_HAS2(Camera, CameraComponent)
			COMPONENT_SWITCH_HAS(NativeScript)
			COMPONENT_SWITCH_HAS(RigidBody2d)
			COMPONENT_SWITCH_HAS(BoxCollider2d)
			default:
				isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Invalid Component")));
				break;
		}
	}

	static void ScriptSuperClassGetComponent(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		AC_PROFILE_FUNCTION();
		v8::Isolate* isolate = args.GetIsolate();
		// v8::HandleScope handle_scope(isolate);

		if (args.Length() != 1)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Wrong number of arguments")));
			return;
		}

		if (!args[0]->IsUint32())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Only Accepts a number")));
			return;
		}

		AC_CORE_TRACE("Getting Component, {}", args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());

		uint16_t component = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
		auto componentEnum = magic_enum::enum_cast<ComponentsEnum>(component);
		if (!componentEnum.has_value())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Invalid Component")));
			return;
		}

		ScriptSuperClass* obj = static_cast<ScriptSuperClass*>(v8::Local<v8::External>::Cast(args.This()->GetInternalField(0))->Value());
		switch (componentEnum.value())
		{
			case ComponentsEnum::Tag:
			{
				if (!obj->HasComponent<Components::Tag>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No Tag Component")));
					return;
				}
				Components::Tag& tagComponent = obj->GetComponent<Components::Tag>();

				try
				{
					v8::Local<v8::Object> tag = v8pp::class_<Components::Tag>::reference_external(isolate, &tagComponent);
					args.GetReturnValue().Set(tag);
				}
				catch (const std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			case ComponentsEnum::Transform:
			{
				AC_CORE_TRACE("Getting Transform");
				if (!obj->HasComponent<Components::Transform>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No Transform Component")));
					return;
				}
				Components::Transform& transformComponent = obj->GetComponent<Components::Transform>();

				try
				{
					//TODO figure out if we need to do this or if we can just class wrap
					//At the moment, class wrap throws an exception "Cannot wrap c++ class"

					v8::Local<v8::Value> pos_ref = V8Vec3::reference_external(isolate, &transformComponent.Translation);
					v8::Local<v8::Value> rot_ref = V8Vec3::reference_external(isolate, &transformComponent.Rotation);
					v8::Local<v8::Value> scale_ref = V8Vec3::reference_external(isolate, &transformComponent.Scale);

					v8::Local<v8::Object> obj = v8::Object::New(isolate);
					v8pp::set_option(isolate, obj, "Position", pos_ref);
					v8pp::set_option(isolate, obj, "Rotation", rot_ref);
					v8pp::set_option(isolate, obj, "Scale", scale_ref);

					args.GetReturnValue().Set(obj);
				}
				catch (std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			case ComponentsEnum::SpriteRenderer:
			{
				if (!obj->HasComponent<Components::SpriteRenderer>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No SpriteRenderer Component")));
					return;
				}
				Components::SpriteRenderer& spriteRendererComponent = obj->GetComponent<Components::SpriteRenderer>();

				try
				{
					v8::Local<v8::Object> obj = v8::Object::New(isolate);

					v8::Local<v8::Value> color_ref = v8pp::class_<glm::vec4>::reference_external(isolate, &spriteRendererComponent.Color);

					v8pp::set_option(isolate, obj, "Color", color_ref);

					args.GetReturnValue().Set(obj);
				}
				catch (std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			//TODO others
			case ComponentsEnum::RigidBody2d:
			{
				if (!obj->HasComponent<Components::RigidBody2d>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No RigidBody2d Component")));
					return;
				}

				Components::RigidBody2d& rigidBody2dComponent = obj->GetComponent<Components::RigidBody2d>();
				try
				{
					v8::Local<v8::Object> v8Obj = v8pp::to_v8(isolate, rigidBody2dComponent);

					AC_CORE_ASSERT(!v8Obj.IsEmpty(), "Failed to convert RigidBody2d");

					args.GetReturnValue().Set(v8Obj);
				}
				catch (std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			case ComponentsEnum::BoxCollider2d:
			{
				if (!obj->HasComponent<Components::BoxCollider2d>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No BoxCollider2d Component")));
					return;
				}

				Components::BoxCollider2d& boxCollider2dComponent = obj->GetComponent<Components::BoxCollider2d>();

				try
				{
					v8::Local<v8::Object> v8Obj = v8pp::to_v8(isolate, boxCollider2dComponent);

					AC_CORE_ASSERT(!v8Obj.IsEmpty(), "Failed to convert BoxCollider2d");

					args.GetReturnValue().Set(v8Obj);
				}
				catch (std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			case ComponentsEnum::CircleCollider2d:
			{
				if (!obj->HasComponent<Components::CircleCollider2d>())
				{
					isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "No CircleCollider2d Component")));
					return;
				}

				Components::CircleCollider2d& circleCollider2dComponent = obj->GetComponent<Components::CircleCollider2d>();

				try
				{
					v8::Local<v8::Object> v8Obj = v8pp::to_v8(isolate, circleCollider2dComponent);

					AC_CORE_ASSERT(!v8Obj.IsEmpty(), "Failed to convert CircleCollider2d");

					args.GetReturnValue().Set(v8Obj);
				}
				catch (std::runtime_error& e)
				{
					AC_CORE_ERROR("{0}", e.what());
				}
			}
			break;
			default:
				isolate->ThrowException(v8::Exception::TypeError(v8pp::to_v8(isolate, "Invalid Component")));
				break;
		}
	}

	//===============================================================================================//
	//											V8Engine											 //
	//===============================================================================================//

	V8Engine::V8Engine()
	{
	}

	void V8Engine::Initialize()
	{
		AC_PROFILE_FUNCTION();
		V8Import::Init();
		// std::string const v8_flags = "--turbo_instruction_scheduling --native-code-counters --expose_gc --print_builtin_code --print_code_verbose --profile_deserialization --serialization_statistics --random-seed 314159265";
		// v8::V8::SetFlagsFromString(v8_flags.data(), (int)v8_flags.length());
		v8::V8::SetFlagsFromString("--random-seed 314159265");

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
		if (!m_KeepRunning)
		{
			v8::V8::Dispose();
			v8::V8::ShutdownPlatform();
			m_Running = false;
			m_Platform.reset();
			V8Import::Save();
		}
	}

	V8Engine::~V8Engine()
	{
		m_KeepRunning = false;
		Shutdown();
	}

	//===============================================================================================//
	//											V8Script											 //
	//===============================================================================================//

	//TODO allow empty methods!
	//TODO add support for other Scripting Methods (onDestroy, onKeyDown,...)
	//TODO add support for async methods? -> If return type is async, add it to a queue that gets resolved after/during other scripts?
	V8Script::V8Script()
	{
		// V8Script(std::string("res/scripts/test.ts"));
	}

	V8Script::V8Script(const std::string& filePath)
		: m_Isolate(NULL)
	{
		AC_CORE_ASSERT(filePath.ends_with(".ts"), "Script must be a typescript file!");
		m_TSFilePath = filePath;
		m_JSFilePath = filePath.substr(0, filePath.length() - 2) + "js";

		s_Scripts.emplace(m_TSFilePath, this);

		m_Data = TSCompiler::Compile(m_TSFilePath);
	}

	V8Script::~V8Script()
	{
		Dispose();

		s_Scripts.erase(s_Scripts.find(m_TSFilePath));
	}

	//TODO ts->js filename interop
	void V8Script::Load(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		V8Engine::instance().AddScript(this);

		v8::Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		m_Isolate = v8::Isolate::New(create_params);

		std::string sourceCode = Utils::File::ReadFile(m_JSFilePath);

		std::string md5Hash = Utils::File::MD5HashString(sourceCode);

		// m_Data = TSCompiler::Compile(m_Isolate, m_TSFilePath);

		AC_CORE_TRACE("TSCompilation succeeded");

		AC_CORE_ASSERT(std::filesystem::exists(m_JSFilePath), "Failed to find compiled script!");

		// m_SnapshotCreator = CreateScope<v8::SnapshotCreator>(m_Isolate, nullptr, snapshot);

		v8::Isolate::Scope isolate_scope(m_Isolate);
		//Block for destroying handle scope before creating startup blob
		{
			// Create a stack-allocated handle scope.
			v8::HandleScope handle_scope(m_Isolate);

			v8::Local<v8::Context> context = CreateShellContext();

			// m_SnapshotCreator->SetDefaultContext(context);

			AC_CORE_TRACE("Serialized Context!");

			// m_Context = context;
			// m_Context.Reset(m_Isolate, context);
			m_Context = v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>>(m_Isolate, context);
			// m_Context = context;

			AC_CORE_ASSERT(!context.IsEmpty(), "Failed to create V8 context!");
			// Enter the context for compiling and running the hello world script.
			v8::Context::Scope context_scope(context);
			{
				// Create a string containing the JavaScript source code.
				v8::Local<v8::String> source =
					v8::String::NewFromUtf8(m_Isolate, sourceCode.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
				// Compile the source code.
				v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

				v8::TryCatch trycatch(m_Isolate);
				v8::MaybeLocal<v8::Value> v = script->Run(context);
				if (v.IsEmpty())
				{
					std::string msg = v8pp::from_v8<std::string>(m_Isolate, trycatch.Exception());
					AC_CORE_ERROR("[V8]: Failed to run script: {0}", msg);
				}

				v8::Local<v8::Value> moduleObject = context->Global()->Get(context, v8::String::NewFromUtf8(m_Isolate, "module").ToLocalChecked()).ToLocalChecked();
				v8::Local<v8::Object> module = v8::Local<v8::Object>::Cast(moduleObject);
				v8::MaybeLocal<v8::Value> maybeExports = module->Get(context, v8::String::NewFromUtf8(m_Isolate, "exports").ToLocalChecked());

				AC_CORE_ASSERT(!maybeExports.IsEmpty(), "Failed to get module exports!");

				v8::Local<v8::Value> exports = maybeExports.ToLocalChecked();

				AC_CORE_ASSERT(exports->IsFunction(), "[V8]: Wrong Export Type!");

				v8::Local<v8::Function> classObj = v8::Local<v8::Function>::Cast(exports);

				m_Name = v8pp::from_v8<std::string>(m_Isolate, classObj->GetDebugName());
				v8::Local<v8::Object> instance = classObj->NewInstance(context).ToLocalChecked();
				ScriptSuperClass* obj = (ScriptSuperClass*)instance->GetInternalField(0).As<v8::External>()->Value();
				obj->m_Entity = entity;

				for (auto& params : m_Parameters)
				{
					v8::Local<v8::String> name = v8pp::to_v8(m_Isolate, params.first);

					v8::Local<v8::Value> value;
					if (params.second.type() == typeid(float))
					{
						value = v8pp::to_v8<float>(m_Isolate, boost::get<float>(params.second));
					}
					else if (params.second.type() == typeid(bool))
					{
						value = v8pp::to_v8<bool>(m_Isolate, boost::get<bool>(params.second));
					}
					else if (params.second.type() == typeid(std::string))
					{
						value = v8pp::to_v8<std::string>(m_Isolate, boost::get<std::string>(params.second));
					}
					else
					{
						AC_CORE_FATAL("[V8]: Unknown parameter type!");
						AC_CORE_BREAK();
					}

					v8::Maybe<bool> result = instance->Set(context, name, value);
					AC_CORE_ASSERT(result.IsJust(), "Failed to set parameter {}!", params.first);
				}

				m_Class.Reset(m_Isolate, instance);

				v8::Local<v8::Function> OnUpdatefunc = instance->Get(context, v8::String::NewFromUtf8(m_Isolate, "OnUpdate").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

				m_OnUpdate.Reset(m_Isolate, OnUpdatefunc);

				v8::Local<v8::Function> OnCreateFunc = instance->Get(context, v8::String::NewFromUtf8(m_Isolate, "OnCreate").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
				auto ret = OnCreateFunc->Call(context, instance, 0, nullptr);
				if (ret.IsEmpty() && trycatch.HasCaught())
				{
					AC_CORE_ERROR("[V8]: Exception {}", v8pp::from_v8<std::string>(m_Isolate, trycatch.Message()->Get()));
				}
				else if (ret.IsEmpty())
				{
					AC_CORE_ERROR("[V8]: Failed to call OnCreate!");
					AC_CORE_BREAK();
				}

				//OnDestroy
				//...
			}
		}

		// v8::StartupData serializedData = m_SnapshotCreator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);

		// AC_CORE_ASSERT(serializedData.data != nullptr, "Failed to create snapshot!");
		// AC_CORE_ASSERT(serializedData.raw_size > 0, "Failed to create snapshot!");

		// AC_CORE_INFO("Writing Snapshot to {}", snapshotPath.string());

		// std::ofstream out(snapshotPath.string(), std::ios::binary);

		// AC_CORE_ASSERT(out, "Failed to write snapshot");

		// out.write(reinterpret_cast<const char*>(serializedData.data), serializedData.raw_size);

		// out.close();

		// AC_CORE_INFO("Snapshot written to {}", snapshotPath.string());
	}

	void V8Script::Dispose()
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_INFO("V8 Isolate {} != {}", (void*)m_Isolate, (void*)NULL);
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
		filewatch::FileWatch<std::string> watch(
			m_TSFilePath,
			[](const std::string& path, const filewatch::Event changeType)
			{
				AC_PROFILE_FUNCTION();
				if (changeType == filewatch::Event::modified)
				{
					AC_CORE_INFO("File {} changed!", path);
					V8Script* script = s_Scripts[path];
					if (script)
					{
						script->Compile();
					}
					else
					{
						AC_CORE_WARN("Modified Event for unknown script {}!", path);
					}
				}
			});
	}

	void V8Script::OnUpdate(Timestep ts, Camera* camera)
	{
		AC_PROFILE_FUNCTION();
		Timer timer;
		AC_CORE_ASSERT(m_Isolate != nullptr, "V8 Isolate is null!");

		v8::Isolate::Scope isolate_scope(m_Isolate);
		v8::HandleScope handle_scope(m_Isolate);

		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(m_Isolate, m_Context);

		v8::Context::Scope context_scope(context);
		{
			AC_CORE_ASSERT(!m_OnUpdate.IsEmpty(), "V8 OnUpdate is null!");
			AC_CORE_ASSERT(!m_Class.IsEmpty(), "V8 Script Instance is null!");
			AC_CORE_ASSERT(!context.IsEmpty(), "V8 Context is null!");

			v8::Local<v8::Value> time = v8::Number::New(m_Isolate, ts.GetSeconds());

			v8::Local<v8::Value> args[1] = {time};
			v8::Local<v8::Function> onUpdate = v8::Local<v8::Function>::New(m_Isolate, m_OnUpdate);
			v8::Local<v8::Object> instance = v8::Local<v8::Object>::New(m_Isolate, m_Class);
			v8::TryCatch tryCatch(m_Isolate);
			v8::MaybeLocal<v8::Value> res = onUpdate->Call(context, instance, 1, args);

			if (tryCatch.HasCaught())
			{
				if (!tryCatch.Message().IsEmpty())
					AC_CORE_ERROR("[V8]: Error on Update: {}", v8pp::from_v8<std::string>(m_Isolate, tryCatch.Message()->Get()));
				else
					AC_CORE_ERROR("[V8]: Unknown Error on Update");
			}

			AC_CORE_ASSERT(!res.IsEmpty(), "V8 OnUpdate failed!");
			AC_CORE_ASSERT(res.ToLocalChecked()->IsUndefined(), "V8 OnUpdate returned a value!");

			m_LastExecutionTime = timer.ElapsedMillis();
		}
	}

	v8::Local<v8::Context> V8Script::CreateShellContext()
	{
		AC_PROFILE_FUNCTION();
		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
		global->Set(v8::String::NewFromUtf8(m_Isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(m_Isolate, Print));

		v8::Local<v8::ObjectTemplate> componentsEnum = v8::ObjectTemplate::New(m_Isolate);
		constexpr auto& components = magic_enum::enum_values<ComponentsEnum>();
		for (const auto& component : components)
		{
			componentsEnum->Set(v8pp::to_v8(m_Isolate, magic_enum::enum_name(component)), v8pp::to_v8(m_Isolate, magic_enum::enum_integer(component)));
		}

		//TODO switch out with string names?
		global->Set(v8pp::to_v8(m_Isolate, "ComponentTypes"), componentsEnum);

		v8::Local<v8::ObjectTemplate> module = v8::ObjectTemplate::New(m_Isolate);
		module->Set(v8pp::to_v8(m_Isolate, "exports"), v8::ObjectTemplate::New(m_Isolate));

		global->Set(v8pp::to_v8(m_Isolate, "module"), module);

		v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(m_Isolate, ScriptSuperClassConstructor);
		v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();
		instance->SetInternalFieldCount(1);
		constructor->PrototypeTemplate()->Set(v8pp::to_v8(m_Isolate, "HasComponent"), v8::FunctionTemplate::New(m_Isolate, ScriptSuperClassHasComponent));
		constructor->PrototypeTemplate()->Set(v8pp::to_v8(m_Isolate, "GetComponent"), v8::FunctionTemplate::New(m_Isolate, ScriptSuperClassGetComponent));
		global->Set(v8pp::to_v8(m_Isolate, "ScriptSuperClass"), constructor);

		DeclareTypes(global);

		v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

		return context;
	}

	void V8Script::DeclareTypes(v8::Local<v8::ObjectTemplate>& global)
	{
		AC_PROFILE_FUNCTION();
		v8::HandleScope handle_scope(m_Isolate);
		v8pp::module mathModule(m_Isolate);

		v8pp::class_<glm::vec2> vec2(m_Isolate);
		vec2.ctor<float, float>()
			.set("x", &glm::vec2::x)
			.set("y", &glm::vec2::y);

		v8pp::class_<glm::vec3> vec3(m_Isolate);
		vec3.ctor<float, float, float>()
			.set("x", &glm::vec3::x)
			.set("y", &glm::vec3::y)
			.set("z", &glm::vec3::z);

		v8pp::class_<glm::vec4> vec4(m_Isolate);
		vec4.ctor<float, float, float, float>()
			.set("x", &glm::vec4::x)
			.set("y", &glm::vec4::y)
			.set("z", &glm::vec4::z)
			.set("w", &glm::vec4::w);

		mathModule.set("vec2", vec2);
		mathModule.set("vec3", vec3);
		mathModule.set("vec4", vec4);

		v8pp::class_<Input> inputClass(m_Isolate);
		inputClass.set("IsKeyPressed", &Input::IsKeyPressed)
			.set("IsMouseButtonPressed", &Input::IsMouseButtonPressed)
			.set("GetMouseX", &Input::GetMouseX)
			.set("GetMouseY", &Input::GetMouseY)
			.set("GetMousePosition", Input::GetMousePosition);

		global->Set(v8pp::to_v8(m_Isolate, "Input"), inputClass.js_function_template());

		global->Set(v8pp::to_v8(m_Isolate, "math"), mathModule.impl());

		v8pp::module componentModule(m_Isolate);

		v8pp::class_<Components::Tag> tag(m_Isolate);
		tag.ctor<std::string>()
			.set("TagName", &Components::Tag::TagName)
			.auto_wrap_objects(true);

		v8pp::class_<Components::RigidBody2d> rigidBody2d(m_Isolate);
		rigidBody2d.ctor<>()
			.set("AddForce", &Components::RigidBody2d::AddForce)
			.auto_wrap_objects(true);

		v8pp::class_<Physics2D::Collider> collider(m_Isolate);
		collider
			.set("Offset", v8pp::property_(&Physics2D::Collider::GetOffset, &Physics2D::Collider::SetOffset))
			.set("IsInside", &Physics2D::Collider::IsInside)
			.auto_wrap_objects(true);

		v8pp::class_<Components::BoxCollider2d> boxCollider(m_Isolate);
		boxCollider.ctor<>()
			.inherit<Physics2D::Collider>()
			.set("Size", v8pp::property_(&Components::BoxCollider2d::GetSize, &Components::BoxCollider2d::SetSize))
			.auto_wrap_objects(true);

		v8pp::class_<Components::CircleCollider2d> circleCollider(m_Isolate);
		circleCollider.ctor<>()
			.inherit<Physics2D::Collider>()
			.set("Radius", v8pp::property_(&Components::CircleCollider2d::GetRadius, &Components::CircleCollider2d::SetRadius))
			.auto_wrap_objects(true);

		componentModule.set("Tag", tag);
		componentModule.set("RigidBody2d", rigidBody2d);
		componentModule.set("Collider", collider); //TODO move to physics module?
		componentModule.set("BoxCollider2d", boxCollider);
		componentModule.set("CircleCollider2d", circleCollider);

		global->Set(v8pp::to_v8(m_Isolate, "Components"), componentModule.impl());
	}

	template <typename T>
	void V8Script::SetValue(std::string parameterName, T value)
	{
		static_assert(false, "Invalid Type");
	}

	template <>
	void V8Script::SetValue<bool>(std::string parameterName, bool value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter")
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Boolean, "Tried to set invalid parameter");
		m_Parameters[parameterName] = value;
	}
	template <>

	void V8Script::SetValue<float>(std::string parameterName, float value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter")
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Number, "Tried to set invalid parameter");
		m_Parameters[parameterName] = value;
	}

	template <>
	void V8Script::SetValue<std::string>(std::string parameterName, std::string value)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to set invalid parameter")
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
			std::string val = boost::apply_visitor(StringVisitor(), parameter.second);
			parameters[parameter.first] = val;
		}
		return parameters;
	}

	void V8Script::SetParameters(std::unordered_map<std::string, std::string> params)
	{
		//TODO move ts parsing somewhere, so that we can use it here for type checking!
		for (auto& parameter : params)
		{
		}
	}

	template <>
	bool& V8Script::GetValue<bool>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter")
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Boolean, "Tried to get invalid parameter for type boolean");
		if (!m_Parameters.contains(parameterName))
		{
			//TODO parse default value from typescript
			m_Parameters[parameterName] = false;
		}
		return boost::get<bool>(m_Parameters[parameterName]);
	}

	template <>
	float& V8Script::GetValue<float>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter")
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::Number, "Tried to get invalid parameter for type number");
		if (!m_Parameters.contains(parameterName))
		{
			//TODO parse default value from typescript
			m_Parameters[parameterName] = 0.0f;
		}
		return boost::get<float>(m_Parameters[parameterName]);
	}

	template <>
	std::string& V8Script::GetValue<std::string>(std::string parameterName)
	{
		AC_CORE_ASSERT(m_Data.Fields.contains(parameterName), "Tried to get invalid parameter")
		TSField field = m_Data.Fields[parameterName];
		AC_ASSERT(field.Type == TsType::String, "Tried to get invalid parameter for type string");
		if (!m_Parameters.contains(parameterName))
		{
			//TODO parse default value from typescript
			m_Parameters[parameterName] = std::string("");
		}
		return boost::get<std::string>(m_Parameters[parameterName]);
	}
}