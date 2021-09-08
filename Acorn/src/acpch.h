#pragma once

#include "core/Core.h"
#include "core/CoreConfig.h"

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

#include "core/Log.h"

#include "debug/Instrumentor.h"

#ifdef AC_PLATFORM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif