
#pragma once

namespace Acorn::IDL
{
	const char* WrapperHeader = R"~~~(#pragma once

#include "Acorn/core/Core.h"

#include <v8.h>

namespace Acorn::Scripting::V8
{
	class {{ name }}Wrapper
	{
public:
		static void Bind(v8::Local<v8::Object> global);


	};
})~~~";
}
			