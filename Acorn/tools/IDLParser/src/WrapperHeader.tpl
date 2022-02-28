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

#pragma once

#include "Acorn/core/Core.h"

#include "{{ acorn_root }}/{{ name }}.h"

#include <v8.h>

namespace Acorn::Scripting::V8
{

	class {{ name }}Wrapper
	{
public:
        friend class {{ name }};
		static void Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global);
		static {{ name }}* Unwrap(v8::Handle<v8::Object> obj);
		static v8::Local<v8::Object> Wrap(v8::Isolate* isolate, {{ name }}* obj);
	};

## for enum in enums
{% include "Enum.tpl" %}
## endfor
}