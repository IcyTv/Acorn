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

#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>

namespace Acorn::Scripting::V8
{
## if typedef_name
	using {{ interface_name }} = {{ typedef_name }};
## endif

## for method in methods
	static void {{ method.name }}Wrapper(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
## for arg in method.args
		{{ arg.type }}& {{ arg.name }} = v8pp::from_v8<{{ arg.type }}&>(args.GetIsolate(), args[{{ loop.index1 - 1 }}]);
## endfor


## if method.return_type != "void"
		{{ method.return_type }} ret = {{ to_pascal_case(method.name) }}(
## for arg in method.args
			, {{ arg.name }}
		);
## endfor
		args.GetReturnValue().Set(v8pp::to_v8(args.GetIsolate(), ret));
## else
		{{ method.cpp_name }}(
## for arg in method.args
			, {{ arg.name }}
## endfor
		);
## endif
	}
## endfor


	void {{ wrapper_name }}::Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global)
	{
		// TODO do we want accessors on global objects?
## for method in methods
		global->Set(v8pp::to_v8(isolate, "{{ method.name }}"), v8::FunctionTemplate::New(isolate, {{ method.name }}Wrapper));
## endfor
	}


	{% include "EnumImplementation.tpl" %}
}