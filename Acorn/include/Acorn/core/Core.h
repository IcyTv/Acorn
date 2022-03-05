
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "CoreConfig.h"
#include "Log.h"

#include <memory>
#include <string_view>

using namespace std::literals; // To allow ""sv construction

#ifdef AC_PLATFORM_WINDOWS
	#ifdef AC_DYNAMIC_LINK
		#ifdef AC_BUILD_DLL
			#define AC_API __declspec(dllexport)
		#else
			#define AC_API__declspec(dllexport)
		#endif
	#elif defined(AC_PLATFORM_LINUX)
		#ifdef AC_BUILD_DLL
			#define AC_API __attribute__((visibility("default")))
		#else
			#define AC_API
		#endif
	#else
		#define AC_API
	#endif
#endif

#define BIT(x) (1 << x)

#ifdef AC_ENABLE_ASSERTS
							   // #include "debug-trap/debug-trap.h"
	#include "utils/debugbreak.h"
	#define AC_ASSERT(x, ...)          \
		{                              \
			if (!(x))                  \
			{                          \
				AC_ERROR(__VA_ARGS__); \
				debug_break();         \
			}                          \
		}
	#define AC_CORE_ASSERT(x, ...)          \
		{                                   \
			if (!(x))                       \
			{                               \
				AC_CORE_ERROR(__VA_ARGS__); \
				debug_break();              \
			}                               \
		}
	#define AC_ASSERT_NOT_REACHED() AC_CORE_ASSERT(false, "Code should not reach here {}:{}!", __FILE__, __LINE__)
#else
	#define AC_ASSERT(x, ...)
	#define AC_CORE_ASSERT(x, ...)
	#define AC_ASSERT_NOT_REACHED()
#endif

#define DISABLE_ALL_WARNINGS_BEGIN \
	__pragma(warning(push, 0))

#define DISABLE_ALL_WARNINGS_END \
	__pragma(warning(pop))

#ifdef AC_DEBUG
							   // #include <debug-trap/debug-trap.h>
	#include "utils/debugbreak.h"
	#define AC_CORE_BREAK() debug_break()
#else
	#define AC_CORE_BREAK()
#endif

namespace Acorn
{
	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}