#pragma once

#include "core/Core.h"

#include <v8.h>

namespace Acorn::Scripting::V8
{
	class Wrapper : public v8::ObjectTemplate
	{
		public:
			virtual void Bind() = 0;

		protected:
			Wrapper() {}
			virtual ~Wrapper() {}
	};

	/*
	 * Retrieve the c++ object pointer from the js object
	 */
	template <typename T>
	T* Unwrap(const v8::Arguments& arg)
	{
		auto self = args.Holder();
		auto wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
		return static_cast<T*>(wrap->Value());
	}

	/**
	 * Construct a new c++ object and wrap it in a js object
	 */
	template <typename T, typename... Args>
	v8::Persistent<v8::Object> make_object(v8::Handle<v8::Object> object, Args&&... args)
	{
		auto x	 = new T(std::forward<Args>(args)...);
		auto obj = v8::Persistent<v8::Object>::New(object);
		obj->SetInternalField(0, v8::External::New(x));

		obj.MakeWeak(
			x,
			[](v8::Persistent<v8::Value> obj, void* data)
			{
				auto x = static_cast<T*>(data);
				delete x;

				obj.Dispose();
				obj.Clear();
			}
		);

		return obj;
	}

	/**
	 * Bind the wrapper object into v8
	 */
	void BindSimple(v8::Local<v8::Object> global)
	{
		auto name = v8::String::NewFromUtf8("Simple");

		auto tpl = v8::FunctionTemplate::New(
					   [&](const v8::Arguments& args) -> v8::Handle<v8::Value>
					   {
						   if (!args.IsConstructCall())
						   {
							   return v8::ThrowException(v8::String::New("Must be called as constructor"));
						   }

						   v8::HandleScope scope;

						   make_object<Wrapper>(args.This());

						   return args.This();
					   }
		)

					   tpl->SetClassName(name);
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		auto proto = tpl->PrototypeTemplate();
		// Add object properties to the prototype
		// Methods, Properties, etc.
	}

} // namespace Acorn::Scripting::V8