#include "acpch.h"

#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Acorn
{
	constexpr uint32_t MAX_FRAMEBUFFER_SIZE = 8192;

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs)
		: m_Specifications(specs), m_RendererId(0)
	{
		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		AC_PROFILE_FUNCTION();
		
		glDeleteFramebuffers(1, &m_RendererId);
		uint32_t textures[] = { m_ColorAttachment, m_DepthAttachment };
		glDeleteTextures(2, textures);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		AC_PROFILE_FUNCTION();
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		glViewport(0, 0, m_Specifications.Width, m_Specifications.Height);
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		AC_PROFILE_FUNCTION();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererId)
		{
			glDeleteFramebuffers(1, &m_RendererId);
			uint32_t textures[] = { m_ColorAttachment, m_DepthAttachment };
			glDeleteTextures(2, textures);
		}

		AC_PROFILE_FUNCTION();

		glCreateFramebuffers(1, &m_RendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specifications.Width, m_Specifications.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specifications.Width, m_Specifications.Height);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		AC_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid Depth Buffer State {}", glCheckFramebufferStatus(GL_FRAMEBUFFER));
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::SetFramebufferSpecs(const FrameBufferSpecs& specs)
	{
		if (specs.Width <= 0 || specs.Height <= 0 || specs.Width > MAX_FRAMEBUFFER_SIZE || specs.Height > MAX_FRAMEBUFFER_SIZE)
		{
			AC_CORE_WARN("Invalid FrameBuffer Size {}, {}! Skipped Resizing", specs.Width, specs.Height);
			return;
		}
		m_Specifications = specs;
		Invalidate();
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width <= 0 || height <= 0 || width > MAX_FRAMEBUFFER_SIZE || height > MAX_FRAMEBUFFER_SIZE)
		{
			AC_CORE_WARN("Invalid FrameBuffer Size {}, {}! Skipped Resizing", width, height);
			return;
		}

		m_Specifications.Width = width;
		m_Specifications.Height = height;
		Invalidate();
	}

}