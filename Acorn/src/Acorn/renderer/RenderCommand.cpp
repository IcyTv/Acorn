#include "acpch.h"

#include "renderer/RenderCommand.h"

#include "platform/opengl/OpenGLRendererApi.h"

namespace Acorn
{
	ACORN_EXPORT Scope<RendererApi> RenderCommand::s_RendererApi = CreateScope<OpenGLRendererApi>();
}