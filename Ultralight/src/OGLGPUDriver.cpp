#include "OGLGPUDriver.h"
#include "Ultralight/Bitmap.h"
#include "Ultralight/platform/GPUDriver.h"
#include "core/Core.h"
#include "renderer/Buffer.h"
#include "renderer/Framebuffer.h"
#include "renderer/RenderCommand.h"
#include "renderer/Texture.h"
#include "renderer/VertexArray.h"

#include <glad/glad.h>
#include <platform/opengl/OpenGLTexture.h>

#include <stdint.h>
#include <unordered_map>

namespace Acorn
{
	static std::unordered_map<uint32_t, Ref<Texture2d>> s_Textures;
	static std::unordered_map<uint32_t, Ref<Framebuffer>> s_Framebuffers;
	static std::unordered_map<uint32_t, Ref<VertexArray>> s_VertexArrays;

	Ref<VertexBuffer> UltralightToAcorn(ultralight::VertexBuffer buffer)
	{
		Ref<VertexBuffer> acornBuffer = VertexBuffer::Create(buffer.size);
		acornBuffer->SetData(buffer.data, buffer.size);
		return acornBuffer;
	}

	Ref<IndexBuffer> UltralightToAcorn(ultralight::IndexBuffer buffer)
	{
		Ref<IndexBuffer> acornBuffer = IndexBuffer::Create((uint32_t*)buffer.data, buffer.size);

		return acornBuffer;
	}

	void OpenGlGPUDriver::BeginDrawing()
	{
		static SceneCamera camera;
		static glm::mat4 view = glm::mat4(1.0f);

		ext2d::Renderer::BeginScene(camera, view);
	}

	void OpenGlGPUDriver::EndDrawing()
	{
		ext2d::Renderer::EndScene();
	}

	void OpenGlGPUDriver::CreateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap)
	{
		if (bitmap->IsEmpty())
		{
			// Create FBO Texture
			return;
		}

		auto pixels = bitmap->LockPixels();
		Ref<Texture2d> texture;

		if (bitmap->format() == ultralight::kBitmapFormat_A8_UNORM)
		{
			texture = Texture2d::Create(bitmap->width(), bitmap->height(), 1);
			texture->SetData(pixels, bitmap->width() * bitmap->height());
		}
		else if (bitmap->format() == ultralight::kBitmapFormat_BGRA8_UNORM_SRGB)
		{
			texture = Texture2d::Create(bitmap->width(), bitmap->height(), 4); // NOTE: is bbp bytes or bits per pixel?
			texture->SetData(pixels, bitmap->width() * bitmap->height() * 4);
		}

		s_Textures[texture_id] = texture;
	}

	void OpenGlGPUDriver::UpdateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap)
	{
		auto texture = s_Textures[texture_id];

		AC_ASSERT(!texture->IsEmpty(), "Texture is empty!");
		AC_ASSERT(texture, "Texture is null!");

		texture->Bind();

		if (!bitmap->IsEmpty())
		{
			void* pixels = bitmap->LockPixels();
			if (bitmap->format() == ultralight::kBitmapFormat_A8_UNORM)
			{
				texture->SetData(pixels, bitmap->width() * bitmap->height());
			}
			else if (bitmap->format() == ultralight::kBitmapFormat_BGRA8_UNORM_SRGB)
			{
				texture->SetData(pixels, bitmap->width() * bitmap->height() * 4);
			}
			else
			{
				AC_ASSERT(false, "Unsupported bitmap format!");
			}
			bitmap->UnlockPixels();
		}
	}

	void OpenGlGPUDriver::BindTexture(uint8_t texture_unit, uint32_t texture_id)
	{
		Ref<Texture2d> texture = Texture2d::FromRenderId(texture_id);
		if (texture)
		{
			texture->Bind(texture_unit);
			BindTexture(texture_unit, texture_id);
		}
	}

	void OpenGlGPUDriver::DestroyTexture(uint32_t textureId)
	{
		s_Textures.erase(textureId);
	}

	void OpenGlGPUDriver::CreateRenderBuffer(uint32_t renderBufferId, const ultralight::RenderBuffer& renderBuffer)
	{
		if (renderBufferId == 0)
		{
			AC_CORE_WARN("Reached usually unreachable code! Buffer 0 is reserved");
			return;
		}

		FrameBufferSpecs specs;

		specs.Width = renderBuffer.width;
		specs.Height = renderBuffer.height;
		specs.Attachments = {
			{FramebufferTextureFormat::RGBA8},
		};

		Ref<Framebuffer> framebuffer = Framebuffer::Create(specs);
		s_Framebuffers[renderBufferId] = framebuffer;
	}

	void OpenGlGPUDriver::BindRenderBuffer(uint32_t renderBufferId)
	{
		Ref<Framebuffer> framebuffer = s_Framebuffers[renderBufferId];
		if (framebuffer)
		{
			framebuffer->Bind();
		}
	}

	void OpenGlGPUDriver::ClearRenderBuffer(uint32_t renderBufferId)
	{
		Ref<Framebuffer> framebuffer = s_Framebuffers[renderBufferId];
		if (framebuffer)
		{
			framebuffer->Bind();
			RenderCommand::Clear();
			framebuffer->Unbind();
		}
	}

	void OpenGlGPUDriver::DestroyRenderBuffer(uint32_t renderBufferId)
	{
		s_Framebuffers.erase(renderBufferId);
	}

	void OpenGlGPUDriver::CreateGeometry(uint32_t geometryId, const ultralight::VertexBuffer& vertices, const ultralight::IndexBuffer& indices)
	{
		if (geometryId == 0)
		{
			AC_CORE_WARN("Reached usually unreachable code! Buffer 0 is reserved");
			return;
		}

		Ref<VertexArray> vertexArray = VertexArray::Create();
		vertexArray->AddVertexBuffer(UltralightToAcorn(vertices));
		vertexArray->SetIndexBuffer(UltralightToAcorn(indices));

		s_VertexArrays[geometryId] = vertexArray;
	}

	void OpenGlGPUDriver::UpdateGeometry(uint32_t geometry_id,
										 const ultralight::VertexBuffer& vertices,
										 const ultralight::IndexBuffer& indices)
	{
		Ref<VertexArray> vertexArray = s_VertexArrays[geometry_id];

		vertexArray->Bind();
		vertexArray->SetVertexBuffer(0, UltralightToAcorn(vertices));
		vertexArray->SetIndexBuffer(UltralightToAcorn(indices));
		vertexArray->Unbind();
	}

	void OpenGlGPUDriver::DrawGeometry(uint32_t geometry_id,
									   uint32_t indices_count,
									   uint32_t indices_offset,
									   const ultralight::GPUState& state)
	{
		Ref<VertexArray> vertexArray = s_VertexArrays[geometry_id];

		RenderCommand::DrawIndexed(vertexArray, indices_count);
	}

	void OpenGlGPUDriver::DestroyGeometry(uint32_t geometry_id)
	{
		s_VertexArrays.erase(geometry_id);
	}

	void OpenGlGPUDriver::DrawCommandList()
	{
	}

	void OpenGlGPUDriver::BindUltralightTexture(uint32_t ultralight_texture_id)
	{
		Ref<Texture2d> texture = Texture2d::FromRenderId(ultralight_texture_id);
		if (texture)
		{
			texture->Bind();
		}
	}

}
