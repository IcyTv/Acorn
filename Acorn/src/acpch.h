#pragma once

#include "core/CoreConfig.h"
#include "core/Core.h"

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>

#include <mutex>

#include "core/Log.h"

#include "debug/Instrumentor.h"

#ifdef AC_PLATFORM_WINDOWS
	#define NOMINMAX
	#include <Windows.h>
#endif