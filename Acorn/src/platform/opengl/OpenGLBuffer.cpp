#include "acpch.h"

#include "debug/Instrumentor.h"
#include "platform/opengl/OpenGLBuffer.h"

#include <glad/glad.h>

// NOTE Since our profiling is linked to the tracy macro, we don't actually need a profiling check...
// Maybe it's worth it to add some custom wrapper macros
#include <TracyOpenGL.hpp>
namespace Acorn
{

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexBuffer::OpenGLVertexBuffer");

		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		AC_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexBuffer::Bind");
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLVertexBuffer::Unbind");

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLVertexBuffer::SetData");

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	const void* OpenGLVertexBuffer::GetData() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLVertexBuffer::GetData");

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		return glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}

	void* OpenGLVertexBuffer::GetDataPtr() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLVertexBuffer::GetDataPtr");

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererId);
		return glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLVertexBuffer::DeleteBuffers");

		glDeleteBuffers(1, &m_RendererId);
	}

	// Index Buffer

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* vertices, uint32_t count)
		: m_Count(count)
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLIndexBuffer::OpenGLIndexBuffer");

		glCreateBuffers(1, &m_RendererId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), vertices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLIndexBuffer::~OpenGLIndexBuffer");

		glDeleteBuffers(1, &m_RendererId);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLIndexBuffer::Bind");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		AC_PROFILE_FUNCTION();

		TracyGpuZone("OpenGLIndexBuffer::Unbind");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}