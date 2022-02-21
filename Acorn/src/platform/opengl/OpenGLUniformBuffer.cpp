#include "acpch.h"

#include "platform/opengl/OpenGLUniformBuffer.h"

#include <glad/glad.h>

#include <TracyOpenGL.hpp>

namespace Acorn
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLUniformBuffer::OpenGLUniformBuffer");

		glCreateBuffers(1, &m_RendererId);
		glNamedBufferData(m_RendererId, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererId);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLUniformBuffer::~OpenGLUniformBuffer");

		glDeleteBuffers(1, &m_RendererId);
	}

	void OpenGLUniformBuffer::Bind()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLUniformBuffer::Bind");

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_RendererId);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLUniformBuffer::SetData");

		glNamedBufferSubData(m_RendererId, offset, size, data);
	}

}