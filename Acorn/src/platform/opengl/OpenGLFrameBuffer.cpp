#include "acpch.h"

#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

namespace Acorn
{
	constexpr uint32_t MAX_FRAMEBUFFER_SIZE = 8192;

	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}
		static void CreateTextures(bool multisampled, uint32_t* outId, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outId);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum format, GLenum internalFormat, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTextureStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				//TODO : Filtering / wrap
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthStencilTexture(uint32_t id, int samples, GLenum format, GLenum type, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTextureStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				//TODO : Filtering / wrap
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, type, TextureTarget(multisampled), id, 0);
		}

		static GLenum AttachmentType(FramebufferTextureFormat type)
		{
			switch (type)
			{
				case FramebufferTextureFormat::RGBA8:
					return GL_RGBA8;
				case FramebufferTextureFormat::R32I:
					return GL_RED_INTEGER;
			}
			AC_CORE_ASSERT(false, "Invalid texture format");
			return 0;
		}

		static bool IsDepthFormat(FBTF format)
		{
			switch (format)
			{
				case FBTF::Depth24Stencil8:
					return true;
				default:
					return false;
			}
		}
	}

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs)
		: m_Specifications(specs), m_RendererId(0)
	{
		for (auto textureSpec : m_Specifications.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(textureSpec.TextureFormat))
			{
				m_ColorAttachmentSpecs.emplace_back(textureSpec);
			}
			else
			{
				AC_CORE_ASSERT(m_DepthAttachmentSpec.TextureFormat == FBTF::None, "Multiple depth attachments are not supported!");
				m_DepthAttachmentSpec = textureSpec;
			}
		}

		Invalidate();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		AC_PROFILE_FUNCTION();

		glDeleteFramebuffers(1, &m_RendererId);

		glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
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

			glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		AC_PROFILE_FUNCTION();

		glCreateFramebuffers(1, &m_RendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

		bool multisample = m_Specifications.Samples > 1;

		//Attachments
		if (m_ColorAttachmentSpecs.size() > 0)
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
					case FBTF::RGBA8:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specifications.Samples, GL_RGBA, GL_RGBA8, m_Specifications.Width, m_Specifications.Height, (int)i);
						break;
					case FBTF::R32I:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Specifications.Samples, GL_RED_INTEGER, GL_R32I, m_Specifications.Width, m_Specifications.Height, (int)i);
						break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FBTF::None)
		{

			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);

			switch (m_DepthAttachmentSpec.TextureFormat)
			{
				case FBTF::Depth24Stencil8:
					Utils::AttachDepthStencilTexture(m_DepthAttachment, m_Specifications.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specifications.Width, m_Specifications.Height);
					break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			AC_CORE_ASSERT(m_ColorAttachments.size() < 4, "Drawbuffers not supported on more than 4 color attachments");
			GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers((GLsizei)m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		AC_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Invalid FrameBuffer State {}", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	int OpenGLFrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererId);
		AC_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Invalid Attachment Index {}", attachmentIndex);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int readPixel = 0;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &readPixel);
		return readPixel;
	}

	void OpenGLFrameBuffer::ClearColorAttachment(uint32_t attachmentIndex, int value)
	{
		AC_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Invalid Attachment Index {}", attachmentIndex);

		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::AttachmentType(spec.TextureFormat), GL_INT, &value);
	}
}