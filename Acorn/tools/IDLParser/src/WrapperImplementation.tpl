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

#include "{{ wrapper_name }}.h"

#include <string_view>
#include <string>

#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>

namespace Acorn::Scripting::V8
{
	void {{ wrapper_name }}::Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global)
	{
		v8pp::class_<{{ name }}> classDef(isolate);
		//TODO readonly
		classDef
			.auto_wrap_objects()
## for constructor in constructors
			.ctor<{{ join(constructor.cpp_args, ", ") }}>()
## endfor
		{% if inherit %}
			.inherit<{{ inherit }}>()
		{% endif %}
## for property in properties
		{% if property.virtual %}
			.set("{{ property.name }}", &{{ name }}::Get{{ to_pascal_case(property.name) }}, &{{ name }}::Set{{ to_pascal_case(property.name) }})
		{% else %}
			.set("{{ property.name }}", &{{ name }}::{{ property.name }}, {{ property.readonly }})
		{% endif %}
## endfor
## for method in methods
			.set("{{ method.name }}", &{{ name }}::{{ to_pascal_case(method.name) }})
## endfor
			;

		v8::Local<v8::FunctionTemplate> tpl = classDef.js_function_template();

		{% if indexed_property_getter %}
		tpl->PrototypeTemplate()->SetIndexedPropertyHandler(
			[](uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info) {
				v8::Isolate* isolate = info.GetIsolate();
				auto ret = v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This())->IndexedGet(index);
				info.GetReturnValue().Set(v8pp::to_v8(isolate, ret));

			},
			{% if indexed_property_setter %}
			[](uint32_t index, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
				v8::Isolate* isolate = info.GetIsolate();
				v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This())->IndexedSet(index, v8pp::from_v8<{{ indexed_property_setter.type }}>(isolate, value));
			}
			{% endif %}
		);
		{% endif %}

		{% if named_property_getter %}
		tpl->PrototypeTemplate()->SetNamedPropertyHandler(
			[](v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
				auto ret = v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This())->NamedGet(v8pp::from_v8<std::string>(isolate, name));
				return v8pp::convert(isolate, ret);
			},
			{% if named_property_setter %}
			[](v8::Isolate* isolate, v8::Local<v8::Object> self, v8::Local<v8::Name> name, v8::Local<v8::Value> value) {
				v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This())->NamedSet(v8pp::from_v8<std::string>(isolate, name), v8pp::from_v8<{{ named_property_setter.type }}>(isolate, value));
			}
			{% endif %}
		);
		{% endif %}

		global->Set(v8pp::to_v8(isolate, "{{ name }}"), tpl);
	}

	{{ name }}& {{ wrapper_name }}::Unwrap(v8::Local<v8::Object> obj)
	{
		v8::Isolate* isolate = v8::Isolate::GetCurrent();
		return Unwrap(isolate, obj);
	}

	{{ name }}& {{ wrapper_name }}::Unwrap(v8::Isolate* isolate, v8::Local<v8::Object> obj)
	{
		return v8pp::from_v8<{{ name }}&>(isolate, obj);
	}

	v8::Local<v8::Object> {{ wrapper_name }}::Wrap(v8::Isolate* isolate, {{ name }}* obj)
	{
		return v8pp::class_<{{ name }}>::reference_external(isolate, obj);
	}
}
