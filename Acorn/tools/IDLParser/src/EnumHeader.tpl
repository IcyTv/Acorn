#pragma once

#include "Acorn/core/Core.h"

namespace Acorn::Scripting::V8
{
	enum class {{ name }}
	{
## for entry in entries
		{{ entry.name }},
## endfor
	};

	inline std::string IDLEnumToString({{ name }} value)
	{
		switch (value)
		{
## for entry in entries
		case {{ name }}::{{ entry.name }}:
			return "{{ entry.string }}";
## endfor
		default:
			return "<unknown>";
		};
	}
}