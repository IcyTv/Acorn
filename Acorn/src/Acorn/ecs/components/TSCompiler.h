#pragma once

#include "core/Core.h"

#include <v8.h>

#include <map>
#include <string>

namespace Acorn
{
	struct TSField
	{
		std::string Name;
		std::string Documentation;
		std::string Type;
	};

	struct TSScriptData
	{
		std::string ClassName;
		std::unordered_map<std::string, TSField> Fields;
	};

	class TSCompiler
	{
	public:
		static TSScriptData Compile(v8::Isolate* isolate, const std::string& code);

	private:
	};
}