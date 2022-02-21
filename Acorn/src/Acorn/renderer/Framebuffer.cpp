#include "acpch.h"

#include "renderer/Framebuffer.h"

#include "platform/opengl/OpenGLFrameBuffer.h"
#include "renderer/Renderer.h"

namespace Acorn
{

	Ref<Framebuffer> Framebuffer::Create(const FrameBufferSpecs& specs)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLFrameBuffer>(specs);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}

}