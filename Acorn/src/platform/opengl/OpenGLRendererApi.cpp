#include "acpch.h"

#include "OpenGLRendererApi.h"

#include <glad/glad.h>

namespace Acorn
{

	void OpenGLRendererApi::Init()
	{
		AC_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST); 
	}

	void OpenGLRendererApi::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererApi::SetClearColor(const glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererApi::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererApi::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count)
	{
		uint32_t indexCount = count ? count: vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	const char* OpenGLRendererApi::GetRenderer() const
	{
		return (const char*)glGetString(GL_RENDERER);
	}

	const char* OpenGLRendererApi::GetVersion() const
	{
		return (const char*)glGetString(GL_VERSION);
	}

	const char* OpenGLRendererApi::GetVendor() const
	{
		return (const char*)glGetString(GL_VENDOR);
	}

}