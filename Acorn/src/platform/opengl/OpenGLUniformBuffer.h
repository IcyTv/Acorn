#pragma once

#include "Acorn/core/Core.h"
#include "Acorn/renderer/UniformBuffer.h"

namespace Acorn
{
	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void Bind() override;

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	private:
		uint32_t m_RendererId;
	};
}