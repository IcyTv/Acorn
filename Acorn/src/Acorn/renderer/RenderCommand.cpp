#include "acpch.h"

#include "RenderCommand.h"

#include "platform/opengl/OpenGLRendererApi.h"

namespace Acorn
{
	Scope<RendererApi> RenderCommand::s_RendererApi = CreateScope<OpenGLRendererApi>();
}