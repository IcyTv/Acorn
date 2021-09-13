#include "acpch.h"

#include "Renderer2D.h"
#include "renderer/BatchRenderer.h"
#include "renderer/RenderCommand.h"
#include "renderer/Shader.h"
#include "renderer/UniformBuffer.h"
#include "renderer/VertexArray.h"

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

		//Editor onyl
		int EntityId = -1;
	};

	struct Renderer2dStorage
	{
		Ref<Texture2d> WhiteTexture;
		Ref<Shader> TextureShader;

		glm::vec4 QuadVertexPositions[4];

		Scope<BatchRenderer<QuadVertex, 6, 4>> BatchRenderer;

		struct CameraData
		{
			glm::mat4 ViewProjection;
			glm::vec4 CameraRight;
			glm::vec4 CameraUp;
		};

		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2dStorage s_Data;

	void Renderer::Init()
	{
		AC_PROFILE_FUNCTION();
		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityId"},
		};

		s_Data.WhiteTexture = Texture2d::Create(1, 1);
		uint32_t white = 0xffffffff;
		s_Data.WhiteTexture->SetData(&white, sizeof(white));

		s_Data.TextureShader = Shader::Create("res/shaders/Textured.shader");

		s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

		std::array<uint32_t, 6> indices = {0, 1, 2, 2, 3, 0};
		s_Data.BatchRenderer = CreateScope<BatchRenderer<QuadVertex, 6, 4>>(s_Data.TextureShader, indices, layout);

		s_Data.TextureShader->Bind();
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2dStorage::CameraData), 0);
	}

	void Renderer::ShutDown()
	{
		AC_PROFILE_FUNCTION();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		AC_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraBuffer.CameraRight = glm::vec4(camera.GetRightDirection(), 0.0f);
		s_Data.CameraBuffer.CameraUp = glm::vec4(camera.GetUpDirection(), 0.0f);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2dStorage::CameraData));

		s_Data.BatchRenderer->Begin();
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		AC_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2dStorage::CameraData));

		s_Data.BatchRenderer->Begin();
	}

	void Renderer::EndScene()
	{
		AC_PROFILE_FUNCTION();

		s_Data.BatchRenderer->End();
	}

	void Renderer::FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		FillQuad({position.x, position.y, 0.0f}, size, color);
	}

	void Renderer::FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		FillQuad(position, size, color, s_Data.WhiteTexture, 1.0f);
	}

	void Renderer::FillQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2d>& texture, float tilingfactor)
	{
		FillQuad({position.x, position.y, 0.0f}, size, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2d>& texture, float tilingfactor)
	{
		FillQuad(position, size, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor)
	{
		FillQuad({position.x, position.y, 0.0f}, size, tint, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor)
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
		FillQuad(transform, tint, texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad({position.x, position.y, 0.0f}, size, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(position, size, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{

		FillQuad({position.x, position.y, 0.0f}, size, tint, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& tint, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		FillQuad(transform, color, s_Data.WhiteTexture);
	}

	void Renderer::FillQuad(const glm::mat4& transform, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(transform, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4& transform, const glm::vec4& tint, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/, int entityId /*= -1*/)
	{
		AC_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;
		const glm::vec2* texCoords = subTexture->GetTexCoords();

		std::array<QuadVertex, quadVertexCount> vertices;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			vertices[i].Position = transform * s_Data.QuadVertexPositions[i];
			vertices[i].Color = tint;
			vertices[i].TexCoord = texCoords[i];
			vertices[i].TexIndex = 0;
			vertices[i].TilingFactor = tilingfactor;
			vertices[i].EntityId = entityId;
		}

		s_Data.BatchRenderer->Draw(subTexture->GetTexture(), vertices);
	}

	void Renderer::FillQuad(const glm::mat4& transform, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/)
	{
		FillQuad(transform, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillQuad(const glm::mat4& transform, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/, int entityId /*= -1*/)
	{
		AC_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 texCoords[] = {
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}};

		std::array<QuadVertex, quadVertexCount> vertices;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			vertices[i].Position = transform * s_Data.QuadVertexPositions[i];
			vertices[i].Color = tint;
			vertices[i].TexCoord = texCoords[i];
			vertices[i].TexIndex = 0;
			vertices[i].TilingFactor = tilingfactor;
			vertices[i].EntityId = entityId;
		}

		s_Data.BatchRenderer->Draw(texture, vertices);
	}

	void Renderer::FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
	}

	void Renderer::FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		FillRotatedQuad(position, size, rotation, color, s_Data.WhiteTexture);
	}

	void Renderer::FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad(position, size, rotation, glm::vec4(1.0f), texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, tint, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor /*= 1.0f*/)
	{

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, texture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad(position, size, rotation, glm::vec4(1.0f), subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		FillRotatedQuad({position.x, position.y, 0.0f}, size, rotation, tint, subTexture, tilingfactor);
	}

	void Renderer::FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<SubTexture>& subTexture, float tilingfactor /*= 1.0f*/)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		FillQuad(transform, tint, subTexture, tilingfactor);
	}

	void Renderer::DrawSprite(const glm::mat4& transform, Components::SpriteRenderer& sprite, int entityId)
	{
		if (sprite.Texture)
		{
			FillQuad(transform, sprite.Color, sprite.Texture, sprite.TilingFactor, entityId);
		}
		else
		{
			FillQuad(transform, sprite.Color, s_Data.WhiteTexture, 1.0f, entityId);
		}
	}

	uint32_t Renderer::GetDrawCalls()
	{
		return s_Data.BatchRenderer->GetStats().DrawCalls;
	}

	uint32_t Renderer::GetQuadCount()
	{
		return s_Data.BatchRenderer->GetStats().QuadCount;
	}

	uint32_t Renderer::GetIndexCount()
	{
		return s_Data.BatchRenderer->GetStats().GetTotalIndexCount();
	}

	uint32_t Renderer::GetVertexCount()
	{
		return s_Data.BatchRenderer->GetStats().GetTotalVertexCount();
	}

	void Renderer::ResetStats()
	{
		s_Data.BatchRenderer->ResetStats();
	}
}