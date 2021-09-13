#include "acpch.h"

#include "V8Script.h"

#include "core/Application.h"
#include "utils/FileUtils.h"

#include <memory>
#include <sstream>

#include <libplatform/libplatform.h>
#include <v8-platform.h>
#include <v8.h>

#include <v8pp/convert.hpp>

void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::stringstream ss;
	for (int i = 0; i < args.Length(); i++)
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		std::string s(*str);
		// AC_CORE_INFO("[V8]: {0}", s);
		ss << s << " ";
	}
	AC_CORE_INFO("[V8]: {0}", ss.str());
}

namespace Acorn
{
	class TestSuperClass
	{
	public:
		TestSuperClass() {}
		~TestSuperClass() {}

		virtual void OnUpdate()
		{
			AC_CORE_INFO("[V8]: TestSuperClass::OnUpdate()");
		}

		void TestMethod(const std::string& msg)
		{
			AC_CORE_INFO("[V8]: TestMethod: {0}", msg);
		}
	};

	void Register(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		AC_CORE_ASSERT(args.Length() == 1, "[V8]: Register function expects 1 argument");
		AC_CORE_ASSERT(args[0]->IsObject(), "[V8]: Register function expects an object as argument");

		v8::Isolate* isolate = args.GetIsolate();
		v8::Local<v8::Context> context = isolate->GetCurrentContext();

		v8::Local<v8::Function> object = args[0].As<v8::Function>();

		// v8::Local<v8::Value> test = object->CallAsConstructor(context, 0, nullptr).ToLocalChecked();
		v8::Local<v8::Object> test = object->NewInstance(context).ToLocalChecked();
		v8::Local<v8::Value> nameV8 = object->GetDebugName();
		std::string name = v8pp::from_v8<std::string>(isolate, nameV8);
		AC_CORE_INFO("Registering class {}", name);

		v8::MaybeLocal<v8::Value> maybeValue = object->Get(context, v8pp::to_v8(isolate, "OnUpdate"));
		AC_CORE_ASSERT(!maybeValue.IsEmpty(), "[V8]: Register function expects an object with a function named OnUpdate");

		v8::Local<v8::Function> function = maybeValue.ToLocalChecked().As<v8::Function>();
		AC_CORE_ASSERT(!function.IsEmpty(), "[V8]: Register function expects an object with a function named OnUpdate");

		// std::string funcName = v8pp::from_v8<std::string>(isolate, function->GetDebugName());
		// AC_CORE_INFO("Registering function {}", funcName);

		v8::MaybeLocal<v8::Value> res = function->Call(context, test, 0, nullptr);
		AC_CORE_ASSERT(!res.IsEmpty(), "[V8]: Register function failed to call OnUpdate");
	}

	V8Engine::V8Engine()
	{
		ApplicationCommandLineArgs args = Application::Get().GetCommandLineArgs();

		v8::V8::InitializeICUDefaultLocation(args.Args[0]);
		v8::V8::InitializeExternalStartupData(args.Args[0]);
		m_Platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(m_Platform.get());
		v8::V8::Initialize();
	}

	V8Engine::~V8Engine()
	{
		v8::V8::Dispose();
		v8::V8::ShutdownPlatform();
	}

	V8Script::V8Script()
	{
		V8Script(std::string("'Hello World'"));
	}

	V8Script::V8Script(const std::string& source)
		: m_Source(source)
	{
		V8Engine::instance().AddScript(this);
		v8::Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		m_Isolate = v8::Isolate::New(create_params);

		AC_CORE_ASSERT(m_Isolate, "Failed to create V8 isolate!");
	}

	V8Script::~V8Script()
	{
		m_Isolate->Dispose();
	}

	void V8Script::Run()
	{
		AC_CORE_ASSERT(m_Isolate != nullptr, "V8 Isolate is null!");

		v8::Isolate::Scope isolate_scope(m_Isolate);
		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(m_Isolate);
		v8::Local<v8::Context> context = CreateShellContext();

		AC_CORE_ASSERT(!context.IsEmpty(), "Failed to create V8 context!");
		// Enter the context for compiling and running the hello world script.
		v8::Context::Scope context_scope(context);
		{
			// Create a string containing the JavaScript source code.
			v8::Local<v8::String> source =
				v8::String::NewFromUtf8(m_Isolate, m_Source.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
			// Compile the source code.
			v8::Local<v8::Script> script =
				v8::Script::Compile(context, source).ToLocalChecked();

			v8::TryCatch trycatch(m_Isolate);
			v8::MaybeLocal<v8::Value> v = script->Run(context);
			if (v.IsEmpty())
			{
				std::string msg = v8pp::from_v8<std::string>(m_Isolate, trycatch.Exception());
				AC_CORE_ERROR("[V8]: Failed to run script: {0}", msg);
			}

			// v8::Local<v8::Object> obj = v8pp::class_<TestSuperClass>::create_object(m_Isolate);

			// TestSuperClass* test = v8pp::from_v8<TestSuperClass*>(m_Isolate, obj);
			// test->OnUpdate();
		}
	}

	v8::Local<v8::Context> V8Script::CreateShellContext()
	{
		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
		global->Set(v8::String::NewFromUtf8(m_Isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(m_Isolate, Print));
		global->Set(v8::String::NewFromUtf8(m_Isolate, "register", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(m_Isolate, Register));

		v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

		return context;

		// v8pp::class_<TestSuperClass> testClass(m_Isolate);
		// testClass
		// 	.ctor<>()
		// 	.set("OnUpdate", &TestSuperClass::OnUpdate)
		// 	.set("TestMethod", &TestSuperClass::TestMethod);
		// global->Set(v8::String::NewFromUtf8(m_Isolate, "TestSuperClass", v8::NewStringType::kNormal).ToLocalChecked(), testClass.js_function_template());

		// // v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);
		// v8pp::context::options options;
		// options.isolate = m_Isolate;
		// options.global = global;
		// v8pp::context context(options);
		// return context;
	}

}