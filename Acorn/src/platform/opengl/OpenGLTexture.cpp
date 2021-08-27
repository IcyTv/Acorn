#include "acpch.h"
#include "OpenGLTexture.h"

#include <stb_image.h>

namespace Acorn
{

	OpenGLTexture2d::OpenGLTexture2d(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
	{
		AC_PROFILE_FUNCTION();


		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2d::OpenGLTexture2d(const std::string& path)
		: m_Path(path)
	{
		AC_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(1);

		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			AC_PROFILE_SCOPE("OpenGLTexture2d::OpenGLTexture2d(const std::string&)::stbi_load");
			data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
		}

		AC_CORE_ASSERT(data, "Failed to load image");

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		AC_CORE_ASSERT(internalFormat & dataFormat, "Format not supported");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2d::~OpenGLTexture2d()
	{
		AC_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTexture2d::SetData(void* data, uint32_t size)
	{
		AC_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		AC_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture");
		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2d::Bind(uint8_t slot) const
	{
		AC_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererId);
	}

}