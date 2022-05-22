#pragma once

#include <Tracy.hpp>

#if AC_PROFILE
	#define AC_PROFILE_FUNCTION() ZoneScoped
	#define AC_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
	#define AC_PROFILE_SCOPE(name)
	#define AC_PROFILE_FUNCTION()
#endif