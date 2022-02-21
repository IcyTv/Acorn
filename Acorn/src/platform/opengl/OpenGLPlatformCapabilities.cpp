#include "acpch.h"

#include "platform/opengl/OpenGLPlatformCapabilities.h"

#include <glad/glad.h>

namespace Acorn
{
	uint32_t OpenGLPlatformCapabilities::GetMaxTextureUnits_()
	{
		int textureUnits = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);
		AC_CORE_ASSERT(textureUnits != 0, "Something went wrong, driver reports no texture support!");
		return textureUnits;
	}
}