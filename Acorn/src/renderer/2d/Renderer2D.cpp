#include "acpch.h"

#include "Renderer2D.h"
#include "renderer/VertexArray.h"
#include "renderer/Shader.h"
#include "renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Acorn::ext2d
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct Renderer2dStorage
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; //Todo Render Capabilities

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2d> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex *QuadVertexBufferBase = nullptr;
		QuadVertex *QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2d>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		glm::vec4 QuadVertexPositions[4];

		Renderer::Statistics Stats;
	};

	static Renderer2dStorage s_Data;

	void Renderer::Init()
	{
		AC_PROFILE_FUNCTION();

		s_Data.QuadVertexArray = VertexArray::Create();

		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexBuffer->SetLayout({
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
		});

		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t *quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;
			offset += 4;
		}

		Ref<IndexBuffer> quadIndexBuffer;
		quadIndexBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);

		delete[] quadIndices;

		s_Data.QuadVertexArray->SetIndexBuffer(quadIndexBuffer);

		s_Data.WhiteTexture = Texture2d::Create(1, 1);
		uint32_t white = 0xffffffff;
		s_Data.WhiteTexture->SetData(&white, sizeof(white));

		int32_t samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_Data.TextureShader = Shader::Create("res/shaders/Textured.shader");

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures[0]", samplers, s_Data.MaxTextureSlots);

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
	}

	void Renderer::ShutDown()
	{
		AC_PROFILE_FUNCTION();
	}

	void Renderer::BeginScene(const OrthographicCamera &camera)
	{
		AC_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.QuadVertexArray->Bind();

		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotIndex = 1;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer::BeginScene(const Camera &camera, const glm::mat4 &transform)
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 viewProjection = camera.GetProjection() * glm::inverse(transform);

		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProjection);

		s_Data.QuadVertexArray->Bind();

		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotIndex = 1;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer::EndScene()
	{
		AC_PROFILE_FUNCTION();

		uint32_t size = (uint32_t)((uint8_t *)s_Data.QuadVertexBufferPtr - (uint8_t *)s_Data.QuadVertexBufferBase);
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, size);

		Flush();
	}

	void Renderer::Flush()
	{
		AC_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}

		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);

		s_Data.Stats.DrawCalls++;
	}

	void Renderer::FlushAndReset()
	{
		EndScene();

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer::FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
	{
		FillQuad({position.x, position.y, 0.0f}, size, color);
	}

	void Renderer::FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
	{
		FillQuad(position, size, color, s_Data.WhiteTexture, 1.0f);
	}

	void Renderer::FillQuad(const glm::vec2 &position, const glm::vec2 &size, const Ref<Texture2d> &texture, float tilingfactor)
	{
		FillQuad({position.x, position.y, 0.0f}, size, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3 &position, const glm::vec2 &size, const Ref<Texture2d> &texture, float tilingfactor)
	{
		FillQuad(position, size, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor)
	{
		FillQuad({position.x, position.y, 0.0f}, size, tint, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor)
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
		FillQuad(transform, tint, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2 &position, const glm::vec2 &size, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad({position.x, position.y, 0.0f}, size, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3 &position, const glm::vec2 &size, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(position, size, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{

		FillQuad({position.x, position.y, 0.0f}, size, tint, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4 &transform, const glm::vec4 &color)
	{
		FillQuad(transform, color, s_Data.WhiteTexture);
	}

	void Renderer::FillQuad(const glm::mat4 &transform, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(transform, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4 &transform, const glm::vec4 &tint, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		AC_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2dStorage::MaxIndices)
		{
			FlushAndReset();
		}

		float textureIndex = 0;

		if (subTexture->GetTexture() != s_Data.WhiteTexture)
		{
			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
			{
				if (*s_Data.TextureSlots[i].get() == *subTexture->GetTexture().get())
				{
					textureIndex = (float)i;
				}
			}

			if (textureIndex == 0)
			{
				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = subTexture->GetTexture();
				s_Data.TextureSlotIndex++;
			}
		}
		constexpr size_t quadVertexCount = 4;
		const glm::vec2 *texCoords = subTexture->GetTexCoords();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tint;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingfactor;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer::FillQuad(const glm::mat4 &transform, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(transform, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4 &transform, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{
		AC_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2dStorage::MaxIndices)
		{
			FlushAndReset();
		}

		constexpr glm::vec4 color = glm::vec4(1.0f);

		float textureIndex = 0;

		if (texture != s_Data.WhiteTexture)
		{
			for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
			{
				if (*s_Data.TextureSlots[i].get() == *texture.get())
				{
					textureIndex = (float)i;
				}
			}

			if (textureIndex == 0)
			{
				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
				s_Data.TextureSlotIndex++;
			}
		}

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 texCoords[] = {
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}};

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = tint;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingfactor;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer::FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
	}

	void Renderer::FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color)
	{
		FillRotatedQuad(position, size, rotation, color, s_Data.WhiteTexture);
	}

	void Renderer::FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad(position, size, rotation, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, tint, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor /*= 1.0f*/)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad(position, size, rotation, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, tint, subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<SubTexture> &subTexture, float tilingfactor /*= 1.0f*/)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, subTexture, tilingfactor);
	}

	Renderer::Statistics Renderer::GetStats()
	{
		return s_Data.Stats;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}
}