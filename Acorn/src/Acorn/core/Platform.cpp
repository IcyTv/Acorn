#include "acpch.h"

#include "core/Platform.h"

#ifdef AC_PLATFORM_WINDOWS
	#include "platform/glfw/GLFWPlatform.h"
#endif

namespace Acorn
{
#ifdef AC_PLATFORM_WINDOWS
	Platform* Platform::s_Platform = new GLFWPlatform();
#endif
}