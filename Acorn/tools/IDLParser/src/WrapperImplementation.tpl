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
#include <sstream>

#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>

namespace Acorn::Scripting::V8
{
## if typedef_name
	using {{ interface_name }} = {{ typedef_name }};
## endif

## for method in methods
## if method.external
	static void {{ method.name }}Wrapper(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		{{ name }}& instance = v8pp::from_v8<{{ name }}&>(args.GetIsolate(), args.Holder());
## for arg in method.args
		{{ arg.type }}& {{ arg.name }} = v8pp::from_v8<{{ arg.type }}&>(args.GetIsolate(), args[{{ loop.index1 - 1 }}]);
## endfor
## if method.return_type != "void"
		{{ method.return_type }} ret = {{ method.external }}(instance
## for arg in method.args
			, {{ arg.name }}
## endfor
	);
		args.GetReturnValue().Set(v8pp::to_v8(args.GetIsolate(), ret));
## else
		{{ method.cpp_name }}(instance
## for arg in method.args
			, {{ arg.name }}
## endfor
				);
## endif
	}
##endif
## endfor

## if stringifier
	static void {{ interface_name }}Stringify(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		const {{ name }}& instance = v8pp::from_v8<{{ name }}&>(args.GetIsolate(), args.Holder());
		std::stringstream ss;
		ss << "{{ interface_name }} {";
## for prop in properties
		ss << "{{ prop.name }}: " << instance.{{ prop.name }} << "{% if not loop.is_last %}, {% endif %}";
## endfor
		ss << "}";
		args.GetReturnValue().Set(v8pp::to_v8(args.GetIsolate(), ss.str()));
	}
## endif

	void {{ wrapper_name }}::Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global)
	{
		v8pp::class_<{{ name }}> classDef(isolate);
		classDef
			.auto_wrap_objects()
## for constructor in constructors
			.ctor<{{ join(constructor.cpp_args, ", ") }}>()
## endfor
## if inherit
		.inherit<{{ inherit }}>()
## endif
## for property in properties
## if property.virtual
			.set("{{ property.name }}", &{{ name }}::Get{{ to_pascal_case(property.name) }}, &{{ name }}::Set{{ to_pascal_case(property.name) }})
## else
			.set("{{ property.name }}", &{{ name }}::{{ property.name }}, {{ property.readonly }})
## endif
## endfor
## for method in methods
## if method.external
			.set("{{ method.name }}", &{{ method.name }}Wrapper)
## else
			.set("{{ method.name }}", &{{ name }}::{{ method.cpp_name }})
## endif
## endfor
## if stringifier
			.set("toString", &{{ interface_name }}Stringify)
## endif
			;

		v8::Local<v8::FunctionTemplate> tpl = classDef.js_function_template();

		{% if indexed_property_getter %}
		tpl->PrototypeTemplate()->SetIndexedPropertyHandler(
			[](uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info) {
				v8::Isolate* isolate = info.GetIsolate();
				const auto& self = v8pp::from_v8<{{ name }}&>(isolate, info.This());
				auto ret = self[index];
				info.GetReturnValue().Set(v8pp::to_v8(isolate, ret));

			},
			{% if indexed_property_setter %}
			[](uint32_t index, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
				v8::Isolate* isolate = info.GetIsolate();
				auto& self = v8pp::from_v8<{{ name }}&>(isolate, info.This());
				self[index] = v8pp::from_v8<{{ indexed_property_setter.type }}>(isolate, value);
			}
			{% endif %}
		);
		{% endif %}

		{% if named_property_getter %}
		tpl->PrototypeTemplate()->SetNamedPropertyHandler(
			[](v8::Local<v8::Name> name, const v8::PropertyCallbackInfo<v8::Value>& info) {
				// auto ret = v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This())->NamedGet(v8pp::from_v8<std::string>(isolate, name));
				v8::Isolate* isolate = info.GetIsolate();
				const auto& self = *v8pp::class_<{{ name }}>::unwrap_object(isolate, info.This());
				auto ret = self[v8pp::from_v8<std::string>(isolate, name)];
				return v8pp::convert(isolate, ret);
			},
			{% if named_property_setter %}
			[](v8::Isolate* isolate, v8::Local<v8::Object> self, v8::Local<v8::Name> name, v8::Local<v8::Value> value) {
				// TODO this does not seem to work...
				auto& self = *v8pp::class_<{{ name }}>::unwrap_object(isolate, self);
				self[v8pp::from_v8<std::string>(isolate, name)] = v8pp::from_v8<{{ named_property_setter.type }}>(isolate, value);
			}
			{% endif %}
		);
		{% endif %}

		global->Set(v8pp::to_v8(isolate, "{{ interface_name }}"), tpl);
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

{% include "EnumImplementation.tpl" %}
}
