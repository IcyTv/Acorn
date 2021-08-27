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

		virtual void SetFramebufferSpecs(const FrameBufferSpecs& specs) override;
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttachmentRendererId() const override { return m_ColorAttachment; };

		inline virtual const FrameBufferSpecs& GetSpecs() const override { return m_Specifications; }

	private:
		uint32_t m_RendererId;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FrameBufferSpecs m_Specifications;
	};
}