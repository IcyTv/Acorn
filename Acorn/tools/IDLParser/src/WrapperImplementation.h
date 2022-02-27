
#pragma once

namespace Acorn::IDL
{
	const char* WrapperImplementation = R"~~~(/**
 * 
 * This file is generated from {{ idl_filename }}.
 *
 * Copyright (c) 2022 Michael Finger
 * 
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 * 
 * For more Information on the license, see the LICENSE.md file
 */

#include "Acorn/Core.h"

#include "{{ acorn_root }}/{{ name }}Wrapper.h"

#include <string_view>
#include <string>

namespace Acorn::Scripting::V8
{
	static v8::Local<v8::Value> Construct{{ name }}(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		//TODO support multiple constructors
		if(!args.IsConstructCall())
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Constructor cannot be called as a function.")));
		}

		if(args.Length() != {{ length(constructor_args) }})
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Constructor expects {{ length(constructor_args) }} arguments.")));
		}

		v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();

## for arg in constructor_args
		if(!args[{{ loop.index1 - 1 }}]->Is{{ arg.v8_type }}())
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument {{ loop.index1 }} is not a {{ arg.v8_type }}.")));
		}
		auto arg{{ loop.index1 }} = args[{{ loop.index1 - 1 }}]->To{{ arg.v8_type }}(context);
## endfor

		//Create object
		{{ name }}* obj = new {{ name }}(
## for arg in constructor_args
			arg{{ loop.index1 }},
## endfor
		);

		args.This()->SetInternalField(0, v8::External::New(args.GetIsolate(), obj));

		return {};
	}

## for method in methods
	static v8::Local<v8::Value> {{ name }}{{ method.name }}(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		v8::Isolate* isolate = args.GetIsolate();
		v8::HandleScope scope(isolate);

		if(args.Length() != {{ length(method.arguments) }})
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("{{ method.name }} expects {{ length(method.arguments) }} arguments.")));
		}

		{{ name }}* obj = {{ name }}Wrapper::Unwrap(args.This());

		if(obj == nullptr)
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("{{ name }} is not a valid object.")));
		}

## for args in method.arguments
		if(!args[{{ loop.index1 - 1 }}]->Is{{ args.v8_type }}())
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Argument {{ loop.index1 }} is not a {{ args.v8_type }}.")));
		}
		auto arg{{ loop.index1 }} = args[{{ loop.index1 - 1 }}]->To{{ args.v8_type }}(isolate->GetCurrentContext());
## endfor

		//Call method
		auto result = obj->{{ method.name }}(
## for args in method.arguments
			arg{{ loop.index1 }},
## endfor
		);

		if(result.IsEmpty())
		{
			return v8::ThrowException(v8::Exception::TypeError(v8::String::New("{{ method.name }} returned an empty result.")));
		}

		return result;
	}
## endfor

	void {{ name }}::Bind(v8::Local<v8::Object> global)
	{
		v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(Construct{{ name }});
		constructor->SetClassName(v8::String::New("{{ name }}"));
		constructor->InstanceTemplate()->SetInternalFieldCount(1);

## for accessor in attributes
		constructor->InstanceTemplate()->SetAccessor(v8::String::New("{{ accessor.name }}"), Get{{ to_pascal_case(accessor.name)}}, Set{{ to_pascal_case(accessor.name) }});
## endfor

## for method in methods
		constructor->PrototypeTemplate()->Set(v8::String::New("{{ method.name }}"), v8::FunctionTemplate::New({{ name }}{{ method.name }}, v8::Signature::New(constructor)));
## endfor

		global->Set(v8::String::New("{{ name }}"), constructor->GetFunction().ToLocalChecked());
	}

	{{ name }}* {{ name }}::Unwrap(v8::Handle<v8::Object> obj)
	{
		v8::Local<v8::Value> value = obj->GetInternalField(0);
		if(value->IsExternal())
		{
			return static_cast<{{ name }}*>(v8::Local<v8::External>::Cast(value)->Value());
		}
		return nullptr;
	}
}
)~~~";
}
			