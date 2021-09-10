#include "acpch.h"

#include "Texture.h"

#include "Renderer.h"
#include "platform/opengl/OpenGLTexture.h"

namespace Acorn
{
	Ref<AsyncTextureLoader> AsyncTextureLoader::Create(const std::string& path, int width, int height)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLAsyncTextureLoader>(path, width, height);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}

	Ref<Texture2d> Texture2d::Create(uint32_t width, uint32_t height, uint32_t bpp)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLTexture2d>(width, height, bpp);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}
	Ref<Texture2d> Texture2d::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLTexture2d>(width, height);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}

	Ref<Texture2d> Texture2d::Create(const std::string& path)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLTexture2d>(path);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}

	Ref<Texture2d> Texture2d::Create(const std::string& path, uint32_t width, uint32_t height)
	{
		switch (Renderer::GetApi())
		{
			case RendererApi::Api::None:
				AC_CORE_ASSERT(false, "RenderApi::None currently not supported");
				return nullptr;
			case RendererApi::Api::OpenGL:
				return CreateRef<OpenGLTexture2d>(path, width, height);
			default:
				AC_CORE_ASSERT(false, "Unknown Renderer Api!");
				return nullptr;
		}
	}
}