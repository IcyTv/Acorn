#include "acpch.h"

#include "FrameProfiler.h"
#include "platform/opengl/OpenGLFrameProfiler.h"

namespace Acorn
{
	Ref<FrameProfiler> FrameProfiler::Create()
	{

		switch (RendererApi::GetAPI())
		{
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLFrameProfiler>();
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer API!");
				return CreateRef<OpenGLFrameProfiler>();
		}
	};

}