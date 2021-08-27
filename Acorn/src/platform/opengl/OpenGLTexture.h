#pragma once

#include "renderer/Texture.h"

#include <glad/glad.h>

namespace Acorn
{
	class OpenGLTexture2d : public Texture2d
	{
	public:
		OpenGLTexture2d(uint32_t width, uint32_t height);
		OpenGLTexture2d(const std::string& path);
		virtual ~OpenGLTexture2d();

		inline virtual uint32_t GetWidth() const override { return m_Width; }
		inline virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void SetData(void* data, uint32_t size) override;

		inline virtual uint32_t GetRendererId() const override { return m_RendererId; }

		virtual void Bind(uint8_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererId == ((OpenGLTexture2d&)other).m_RendererId;
		}
	private:
		std::string m_Path;
		uint32_t m_RendererId;
		uint32_t m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;
	};
}