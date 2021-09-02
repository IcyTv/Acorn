#pragma once

#include "renderer/Framebuffer.h"

namespace Acorn
{

	class OpenGLFrameBuffer : public Framebuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecs& specs);
		virtual ~OpenGLFrameBuffer();

		void Invalidate();

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttachmentRendererId(size_t index) const override
		{
			AC_CORE_ASSERT(index < m_ColorAttachments.size(), "Invalid Color attachment index");
			return m_ColorAttachments[index];
		};

		inline virtual const FrameBufferSpecs& GetSpecs() const override { return m_Specifications; }

	private:
		uint32_t m_RendererId;
		FrameBufferSpecs m_Specifications;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec;

		uint32_t m_DepthAttachment = 0;
		std::vector<uint32_t> m_ColorAttachments;
	};
}