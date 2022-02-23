#include "acpch.h"

#include "ecs/components/TSCompiler.h"

#include "core/Core.h"
#include "core/Log.h"
#include "debug/Instrumentor.h"
#include "ecs/components/V8Script.h"
#include "utils/FileUtils.h"
#include "utils/v8/V8Import.h"

// #include "swc.hpp"

#include "v8pp/convert.hpp"
#include "v8pp/module.hpp"
#include "v8pp/object.hpp"

#include <magic_enum.hpp>
#include <utility>
#include <v8.h>
#include <v8pp/context.hpp>

// Based on https://gist.github.com/surusek/4c05e4dcac6b82d18a1a28e6742fc23e

namespace Acorn
{

	TSScriptData TSCompiler::Compile(const std::string& filepath)
	{
		AC_PROFILE_FUNCTION();

		// AC_ASSERT_NOT_REACHED();

#if 0

		AC_CORE_ASSERT(std::filesystem::exists(filepath), "File not found");

		{
			AC_PROFILE_SCOPE("Swc Compilation");
			bool success = Acorn::compile(reinterpret_cast<const int8_t*>(filepath.c_str()));
			AC_CORE_ASSERT(success, "Compilation failed!");
		}

		TSScriptData data;
		{
			AC_PROFILE_SCOPE("Swc GetTypes");
			ClassParamArrayTuple res = Acorn::getTypes(reinterpret_cast<const int8_t*>(filepath.c_str()));
			AC_CORE_ASSERT(res.ClassParams, "Failed to get types!"); // Nullptr if failed

			AC_CORE_INFO("ClassParameters: {} {}", (void*)res.ClassParams, res.Length);

			for (size_t i = 0; i < res.Length; i++)
			{
				ClassParam item = res.ClassParams[i];
				// AC_CORE_INFO("Found public type: {}", std::string((const char*)item.Name));

				TSField field;
				switch (item.TsType)
				{
					case AcornTsType::Unknown:
						AC_CORE_INFO("Unknown type: {}", std::string((const char*)item.Name));
						break;
					case AcornTsType::Boolean:
						field = TSField{
							std::string((const char*)item.Name),
							"",
							TsType::Boolean,
						};
						break;
					case AcornTsType::Number:
						field = TSField{
							std::string((const char*)item.Name),
							"",
							TsType::Number,
						};
						break;
					case AcornTsType::String:
						field = TSField{
							std::string((const char*)item.Name),
							"",
							TsType::String,
						};
						break;
					case AcornTsType::BigInt:
						field = TSField{
							std::string((const char*)item.Name),
							"",
							TsType::BigInt,
						};
						break;
				};

				data.Fields.emplace(std::string((const char*)item.Name), field);
			}
		}
		return data;
#else
		return {};
#endif
	}
}