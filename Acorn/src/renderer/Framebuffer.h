#pragma once

#include "core/Core.h"

namespace Acorn
{
	struct FrameBufferSpecs
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void SetFramebufferSpecs(const FrameBufferSpecs& specs) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetColorAttachmentRendererId() const = 0;

		virtual const FrameBufferSpecs& GetSpecs() const = 0;

		static Ref<Framebuffer> Create(const FrameBufferSpecs& specs);

	private:

	};

}