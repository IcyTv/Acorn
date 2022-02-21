#pragma once

#include "Acorn/core/Core.h"
#include "Acorn/core/CoreConfig.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

#include <deque>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <mutex>

#if AC_PROFILE
	#include <Tracy.hpp>
#endif

#include "Acorn/core/Log.h"

#include "Acorn/debug/Instrumentor.h"

#ifdef AC_PLATFORM_WINDOWS
	#define NOMINMAX
	#define _WINSOCKAPI_
	#include <Windows.h>
#endif