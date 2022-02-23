#include "acpch.h"

#include "core/Platform.h"

#if defined(AC_PLATFORM_WINDOWS) || defined(AC_PLATFORM_LINUX)
	#include "platform/glfw/GLFWPlatform.h"
#endif

namespace Acorn
{
#if defined(AC_PLATFORM_WINDOWS) || defined(AC_PLATFORM_LINUX)
	Platform* Platform::s_Platform = new GLFWPlatform();
#endif
}