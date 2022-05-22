#pragma once

#include "Acorn/renderer/Texture.h"

namespace Acorn
{

	class OpenGLTexture2d : public Texture2d
	{
	public:
		OpenGLTexture2d(uint32_t width, uint32_t height, uint32_t bpp);
		OpenGLTexture2d(uint32_t width, uint32_t height);
		OpenGLTexture2d(const std::string& path, uint32_t width, uint32_t height);
		OpenGLTexture2d(const std::string& path);
		OpenGLTexture2d(uint32_t rendererId);

		virtual ~OpenGLTexture2d();

		inline virtual uint32_t GetWidth() const override { return m_Width; }
		inline virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetSubData(void* data, uint32_t dataSize, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		inline virtual uint32_t GetRendererId() const override { return m_RendererId; }

		inline virtual std::string GetPath() const override { return m_Path; }

		virtual void SetTextureFiltering(TextureFiltering filtering) override;

		virtual void Bind(uint8_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererId == ((OpenGLTexture2d&)other).m_RendererId;
		}

		static OpenGLTexture2d FromRenderId(uint32_t id);

	private:
		std::string m_Path;
		uint32_t m_RendererId;
		uint32_t m_Width, m_Height;
		uint32_t m_InternalFormat, m_DataFormat;
	};

	class OpenGLAsyncTextureLoader : public AsyncTextureLoader
	{
	public:
		OpenGLAsyncTextureLoader(const std::string& path, int width, int height);

		void Load();

		Ref<Texture2d> Upload();

	private:
		unsigned char* m_Data;
		std::string m_Path;
		int m_Width, m_Height, m_Channels;
		Ref<OpenGLTexture2d> m_Texture;
	};
}