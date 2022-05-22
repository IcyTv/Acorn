
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
#define AC_EXPAND_MACRO(x) x
#define AC_STRINGIFY(x) #x

#ifdef AC_ENABLE_ASSERTS
							   // #include "debug-trap/debug-trap.h"
#include "utils/debugbreak.h"

#define AC_BREAK() debug_break()

template<typename ... Args>
constexpr void AcAssertImpl(std::string_view check, bool checkVal, const char* message = nullptr, Args&& ... args) {
	if(!checkVal){
		if(message == nullptr)
			AC_ERROR("Assertion failed: {0}", check);
		else
			AC_ERROR(message, std::forward<Args>(args) ...);
		AC_BREAK();
	}
}


template<typename ... Args>
constexpr void AcCoreAssertImpl(std::string_view check, bool checkVal, const char* message = nullptr, Args&& ... args) {
	if(!checkVal){
		if(message == nullptr)
			AC_CORE_ERROR("Assertion failed: {0}", check);
		else
			AC_CORE_ERROR(message, std::forward<Args>(args)...);
		AC_BREAK();
	}
}

#define AC_CORE_ASSERT(x, ...) AcCoreAssertImpl(AC_STRINGIFY(x), x, __VA_ARGS__);
#define AC_ASSERT(x, ...) AcAssertImpl(AC_STRINGIFY(x), x, __VA_ARGS__);

#define AC_ASSERT_NOT_REACHED() AC_CORE_ASSERT(false, "Code should not reach here {}:{}!", __FILE__, __LINE__)
#else
#define AC_ASSERT(...)
#define AC_CORE_ASSERT(...)
#define AC_ASSERT_NOT_REACHED()
#endif

#define DISABLE_ALL_WARNINGS_BEGIN __pragma(warning(push, 0))

#define DISABLE_ALL_WARNINGS_END __pragma(warning(pop))

#ifdef AC_DEBUG
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

} // namespace Acorn

// TODO move

#define V8PP_ISOLATE_DATA_SLOT 1