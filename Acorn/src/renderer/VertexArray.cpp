#include "acpch.h"

#include "VertexArray.h"
#include "Renderer.h"

#include "platform/opengl/OpenGLVertexArray.h"

namespace Acorn
{

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetApi())
		{
		case RendererApi::Api::None:
			AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
			return nullptr;
		case RendererApi::Api::OpenGL:
			return CreateRef<OpenGLVertexArray>();
		default:
			AC_CORE_ASSERT(false, "Unknown Renderer Api!");
			return nullptr;
		}
	}

}