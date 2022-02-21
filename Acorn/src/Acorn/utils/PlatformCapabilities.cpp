#include "acpch.h"

#include "utils/PlatformCapabilities.h"

#include "platform/opengl/OpenGLPlatformCapabilities.h"
#include "renderer/Renderer.h"

namespace Acorn
{
	PlatformCapabilities* PlatformCapabilities::s_Instance = nullptr;

	void PlatformCapabilities::Init()
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "Not supported");
				break;
			case RendererApi::Api::OpenGL:
				s_Instance = new OpenGLPlatformCapabilities();
				break;
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer API");
				break;
		}
	}
}