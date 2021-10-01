#include "acpch.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

#include <TracyOpenGL.hpp>

namespace Acorn
{
	OpenGLAsyncTextureLoader::OpenGLAsyncTextureLoader(const std::string& path, int width, int height)
		: m_Path(path), m_Width(width), m_Height(height)
	{
	}

	void OpenGLAsyncTextureLoader::Load()
	{
		AC_PROFILE_FUNCTION();
		stbi_set_flip_vertically_on_load(true);
		if (m_Width < 0 && m_Height < 0)
		{
			m_Data = stbi_load(m_Path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
		}
		else
		{
			int width, height;
			stbi_uc* data = nullptr;

			m_Data = new stbi_uc[m_Width * m_Height * m_Channels];

			data = stbi_load(m_Path.c_str(), &width, &height, &m_Channels, 0);
			stbir_resize_uint8(data, width, height, 0, m_Data, m_Width, m_Height, 0, m_Channels);
		}
	}

	Ref<Texture2d> OpenGLAsyncTextureLoader::Upload()
	{
		AC_PROFILE_FUNCTION();
		Ref<Texture2d> texture = Texture2d::Create(m_Width, m_Height, m_Channels);
		texture->SetData(m_Data, m_Width * m_Height * m_Channels);
		return texture;
	}

	OpenGLTexture2d::OpenGLTexture2d(uint32_t width, uint32_t height, uint32_t bpp)
		: m_Width(width), m_Height(height)
	{
		AC_PROFILE_FUNCTION();
		if (bpp == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		else if (bpp == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else if (bpp == 1)
		{
			m_InternalFormat = GL_R8;
			m_DataFormat = GL_RED;
		}
		else
		{
			AC_CORE_ASSERT(false, "Unsupported texture format!");
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}
		TracyGpuZone("OpenGLTexture2d::OpenGLTexture2d");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (bpp == 1)
		{
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
			glTextureParameteriv(m_RendererId, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		}
	}

	OpenGLTexture2d::OpenGLTexture2d(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_InternalFormat(GL_RGBA8), m_DataFormat(GL_RGBA)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::OpenGLTexture2d");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2d::OpenGLTexture2d(const std::string& path, uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_Path(path)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::OpenGLTexture2d");

		stbi_set_flip_vertically_on_load(1);

		int origWidth, origHeight, channels;
		stbi_uc* data = nullptr;
		{
			AC_PROFILE_SCOPE("OpenGLTexture2d::OpenGLTexture2d(const std::string&)::stbi_load");
			data = stbi_load(m_Path.c_str(), &origWidth, &origHeight, &channels, 0);
		}

		AC_CORE_ASSERT(data, "Failed to load texture!");

		stbi_uc* resizedData = new stbi_uc[m_Width * m_Height * channels];
		stbir_resize_uint8(data, origWidth, origHeight, 0, resizedData, m_Width, m_Height, 0, channels);

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
		// glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, resizedData);

		stbi_image_free(data);
		stbi_image_free(resizedData);
	}

	OpenGLTexture2d::OpenGLTexture2d(const std::string& path)
		: m_Path(path)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::OpenGLTexture2d");

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

		AC_CORE_INFO("Loading Texture with {} channels", channels);

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		AC_CORE_ASSERT(internalFormat & dataFormat, "Format not supported");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2d::~OpenGLTexture2d()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::~OpenGLTexture2d");

		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTexture2d::SetData(void* data, uint32_t size)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::SetData");

		// uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		uint32_t bpp = 4;
		if (m_DataFormat == GL_RGB)
		{
			bpp = 3;
		}
		else if (m_DataFormat == GL_RED)
		{
			bpp = 1;
		}
		AC_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture");
		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2d::SetSubData(void* data, uint32_t dataSize, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::SetSubData");

		uint32_t bpp = 4;
		if (m_DataFormat == GL_RGB)
		{
			bpp = 3;
		}
		else if (m_DataFormat == GL_RED)
		{
			bpp = 1;
		}
		AC_CORE_ASSERT(dataSize == width * height * bpp, "Data must be entire subtexture");
		glTextureSubImage2D(m_RendererId, 0, x, y, width, height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2d::SetTextureFiltering(TextureFiltering filtering)
	{
		GLenum textureFiltering = GL_NEAREST;
		switch (filtering)
		{
			case TextureFiltering::Nearest:
				textureFiltering = GL_NEAREST;
				break;
			case TextureFiltering::Linear:
				textureFiltering = GL_LINEAR;
				break;
			default:
				AC_CORE_ASSERT(false, "Unknown texture filtering");
		}

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, textureFiltering);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, textureFiltering);
	}

	void OpenGLTexture2d::Bind(uint8_t slot) const
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLTexture2d::Bind");

		glBindTextureUnit(slot, m_RendererId);
	}

}