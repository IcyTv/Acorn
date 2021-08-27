#include "acpch.h"

#include "Platform.h"

#ifdef AC_PLATFORM_WINDOWS
#include "platform/windows/WindowsPlatform.h"
#endif

namespace Acorn
{
#ifdef AC_PLATFORM_WINDOWS
	Platform* Platform::s_Platform = new WindowsPlatform();
#endif
}