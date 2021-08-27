#include "acpch.h"

#include "Buffer.h"
#include "Renderer.h"

#include "platform/opengl/OpenGLBuffer.h"

namespace Acorn
{

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetApi())
		{
		case RendererApi::Api::None:
			AC_CORE_ASSERT(false, "RendererAPI::None Not implemented yet!");
			return nullptr;
		case RendererApi::Api::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
		default:
			AC_CORE_ASSERT(false, "Not implemented yet!");
			return nullptr;
		}

	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetApi())
		{
		case RendererApi::Api::None:
			AC_CORE_ASSERT(false, "RendererAPI::None Not implemented yet!");
			return nullptr;
		case RendererApi::Api::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, size);
		default:
			AC_CORE_ASSERT(false, "Not implemented yet!");
			return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetApi())
		{
		case RendererApi::Api::None:
			AC_CORE_ASSERT(false, "RendererAPI::None Not implemented yet!");
			return nullptr;
		case RendererApi::Api::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
		default:
			AC_CORE_ASSERT(false, "Not implemented yet!");
			return nullptr;
		}
	}
}