#include "acpch.h"

#include "Framebuffer.h"

#include "renderer/Renderer.h"
#include "platform/opengl/OpenGLFrameBuffer.h"

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