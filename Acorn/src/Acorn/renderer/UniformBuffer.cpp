#include "acpch.h"

#include "renderer/UniformBuffer.h"

#include "core/Core.h"
#include "platform/opengl/OpenGLUniformBuffer.h"
#include "renderer/Renderer.h"

namespace Acorn
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLUniformBuffer>(size, binding);
			default:
				AC_CORE_ASSERT(false, "Unknown RendererAPI!");
				return nullptr;
		}
	}

}