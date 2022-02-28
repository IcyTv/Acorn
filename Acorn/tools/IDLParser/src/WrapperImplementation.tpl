/**
 * 
 * This file is generated from {{ idl_filename }}.
 *
 * Copyright (c) 2022 Michael Finger
 * 
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 * 
 * For more Information on the license, see the LICENSE.md file
 */

#include "{{ name }}Wrapper.h"

#include <string_view>
#include <string>

namespace Acorn::Scripting::V8
{
	static void Construct{{ name }}(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		auto isolate = args.GetIsolate();

		//TODO support multiple constructors
		if(!args.IsConstructCall())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Constructor cannot be called as a function.").ToLocalChecked()));
		}

		if(args.Length() != {{ length(constructor_args) }})
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Constructor expects {{ length(constructor_args) }} arguments.").ToLocalChecked()));
		}

		v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();

## for arg in constructor_args
		if(!args[{{ loop.index1 - 1 }}]->Is{{ arg.v8_type }}())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Argument {{ loop.index1 }} is not a {{ arg.v8_type }}.").ToLocalChecked()));
		}
		auto arg{{ loop.index1 }} = v8::Local<v8::{{ arg.v8_type }}>::Cast(args[{{ loop.index1 - 1 }}])->Value();
## endfor

		//Create object
		{{ name }}* obj = new {{ name }}({{ join_with_range(", ", "arg{}", length(constructor_args)) }});

		args.This()->SetInternalField(0, v8::External::New(args.GetIsolate(), obj));
	}

## for method in methods
	static void {{ name }}{{ to_pascal_case(method.name) }}(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope scope(isolate);

		if(args.Length() != {{ length(method.args) }})
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ method.name }} expects {{ length(method.args) }} arguments.").ToLocalChecked()));
		}

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(args.This());

		if(obj == nullptr)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ name }} is null.").ToLocalChecked()));
		}

## for arg in method.args
		if(!args[{{ loop.index1 - 1 }}]->Is{{ arg.v8_type }}())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Argument {{ loop.index1 }} is not a {{ arg.v8_type }}.").ToLocalChecked()));
		}
		auto arg{{ loop.index1 }} = v8::Local<v8::{{ arg.v8_type }}>::Cast(args[{{ loop.index1 - 1 }}])->Value();
## endfor

		//Call method
		auto result = obj->{{ to_pascal_case(method.name) }}({{ join_with_range(", ", "arg{}", length(method.args)) }});
		auto returnV8Value = v8::{{ method.return_v8_type }}::New(isolate, result);

		args.GetReturnValue().Set(returnV8Value);
	}
## endfor

## for accessor in attributes
	static void Get{{ to_pascal_case(accessor.name) }}(v8::Local<v8::String> _property, const v8::PropertyCallbackInfo<v8::Value>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(info.This());

		if(obj == nullptr)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ name }} is null.").ToLocalChecked()));
			return;
		}

		auto result = obj->{{ accessor.name }};
		auto returnV8Value = v8::{{ accessor.v8_type }}::New(isolate, result);

		info.GetReturnValue().Set(returnV8Value);
	}

	static void Set{{ to_pascal_case(accessor.name) }}(v8::Local<v8::String> _property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(info.This());

		if(obj == nullptr)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ name }} is null.").ToLocalChecked()));
			return;
		}

		if(!value->Is{{ accessor.v8_type }}())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ accessor.name }} is not a {{ accessor.v8_type }}.").ToLocalChecked()));
		}

		auto arg = v8::Local<v8::{{ accessor.v8_type }}>::Cast(value)->Value();

		obj->{{ accessor.name }} = arg;
	}

## endfor


{% if indexed_property_getter %}
	static void {{ name }}IndexedGet(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(args.This());

		if(obj == nullptr)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ name }} is null.").ToLocalChecked()));
			return;
		}

		try {
			auto result = obj->IndexedGet(index);
			auto returnV8Value = v8::{{ indexed_property_getter.return_v8_type }}::New(isolate, result);

			args.GetReturnValue().Set(returnV8Value);
		} catch(std::out_of_range&) {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Index out of range.").ToLocalChecked()));
		}
	}
{% endif %}

{% if indexed_property_setter %}
	static void {{ name }}IndexedSet(uint32_t index, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)
	{
		v8::Isolate* isolate = info.GetIsolate();

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(info.This());

		if(obj == nullptr)
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "{{ name }} is null.").ToLocalChecked()));
			return;
		}

		if(!value->Is{{ indexed_property_setter.value_v8_type }}())
		{
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Argument for indexed set is not a {{ indexed_property_setter.value_v8_type }}.").ToLocalChecked()));
		}

		auto arg = v8::Local<v8::{{ indexed_property_setter.value_v8_type }}>::Cast(value)->Value();

		try {
			obj->IndexedSet(index, arg);
		} catch(std::out_of_range&) {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Index out of range.").ToLocalChecked()));
		}
	}

{% endif %}


	void {{ name }}Wrapper::Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global)
	{
		auto context = isolate->GetCurrentContext();
		v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(isolate, Construct{{ name }});
		constructor->SetClassName(v8::String::NewFromUtf8(isolate, "{{ name }}").ToLocalChecked());
		constructor->InstanceTemplate()->SetInternalFieldCount(1);

## for accessor in attributes
		constructor->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "{{ accessor.name }}").ToLocalChecked(), Get{{ to_pascal_case(accessor.name)}}, Set{{ to_pascal_case(accessor.name) }});
## endfor

## for method in methods
		constructor->PrototypeTemplate()->Set(v8::String::NewFromUtf8(isolate, "{{ method.name }}").ToLocalChecked(), v8::FunctionTemplate::New(isolate, {{ name }}{{ to_pascal_case(method.name) }}));
## endfor

{% if indexed_property_getter %}
		constructor->PrototypeTemplate()->SetIndexedPropertyHandler(
			{{ name }}IndexedGet
{% if indexed_property_setter %},
			{{ name }}IndexedSet
{% endif %}
		);
{% endif %}

		global->Set(v8::String::NewFromUtf8(isolate, "{{ name }}").ToLocalChecked(), constructor);
	}

	{{ name }}* {{ name }}Wrapper::Unwrap(v8::Handle<v8::Object> obj)
	{
		v8::Local<v8::Value> value = obj->GetInternalField(0);
		if(value->IsExternal())
		{
			return static_cast<{{ name }}*>(v8::Local<v8::External>::Cast(value)->Value());
		}
		return nullptr;
	}

	v8::Local<v8::Object> {{ name }}Wrapper::Wrap(v8::Isolate* isolate, {{ name }}* obj)
	{
		v8::EscapableHandleScope scope(isolate);
		v8::Local<v8::Object> result = v8::Object::New(isolate);
		v8::Local<v8::External> external = v8::External::New(isolate, obj);
		result->SetInternalField(0, external);
		return scope.Escape(result);
	}
}
