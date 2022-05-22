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

#include "{{ acorn_root }}/{{ interface_name }}.h"

#include <v8.h>

namespace Acorn::Scripting::V8
{

	class {{ wrapper_name }}
	{
public:
		static void Bind(v8::Isolate* isolate, v8::Local<v8::ObjectTemplate> global);
	};

{% include "EnumHeader.tpl" %}
}