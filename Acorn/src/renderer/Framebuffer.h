#pragma once

#include "core/Core.h"

namespace Acorn
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		//Color
		RGBA8,
		R32I,

		//Depthstencil
		Depth24Stencil8,

		//Defaults
		Depth = Depth24Stencil8,
	};

	typedef FramebufferTextureFormat FBTF;

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		//TODO filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecs
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;
		FramebufferAttachmentSpecification Attachments;
		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearColorAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetColorAttachmentRendererId(size_t index = 0) const = 0;

		virtual const FrameBufferSpecs& GetSpecs() const = 0;

		static Ref<Framebuffer> Create(const FrameBufferSpecs& specs);

	private:
	};

}