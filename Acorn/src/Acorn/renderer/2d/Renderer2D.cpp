#include "acpch.h"

#include "renderer/2d/Renderer2D.h"
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

		// Editor only
		int EntityId = -1;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		int EntityId;
	};

	struct Renderer2dStorage
	{
		Ref<Texture2d> WhiteTexture;
		Ref<Shader> TextureShader;
		Ref<Shader> CircleShader;

		glm::vec4 QuadVertexPositions[4];

		Scope<BatchRenderer<QuadVertex, 6, 4>> QuadRenderer;
		Scope<BatchRenderer<CircleVertex, 6, 4>> CircleRenderer;

		struct CameraData
		{
			glm::mat4 ViewProjection;
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

		s_Data.TextureShader = Shader::Create(Acorn::Utils::File::ResolveResPath("res/shaders/Textured.shader"));

		s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

		std::array<uint32_t, 6> indices = {0, 1, 2, 2, 3, 0};
		s_Data.QuadRenderer = CreateScope<BatchRenderer<QuadVertex, 6, 4>>(s_Data.TextureShader, indices, layout);
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2dStorage::CameraData), 0);

		BufferLayout circleLayout = {
			{ShaderDataType::Float3, "a_WorldPosition"},
			{ShaderDataType::Float3, "a_LocalPosition"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float, "a_Thickness"},
			{ShaderDataType::Float, "a_Fade"},
			{ShaderDataType::Int, "a_EntityId"},
		};

		s_Data.CircleShader = Shader::Create(Acorn::Utils::File::ResolveResPath("res/shaders/Circle.shader"));
		s_Data.CircleRenderer = CreateScope<BatchRenderer<CircleVertex, 6, 4>>(s_Data.CircleShader, indices, circleLayout);
	}

	void Renderer::ShutDown()
	{
		AC_PROFILE_FUNCTION();

		s_Data.WhiteTexture.reset();
		s_Data.TextureShader.reset();
		s_Data.CircleShader.reset();

		s_Data.QuadRenderer.reset();
		s_Data.CircleRenderer.reset();

		s_Data.CameraUniformBuffer.reset();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		AC_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2dStorage::CameraData));

		s_Data.QuadRenderer->Begin();
		s_Data.CircleRenderer->Begin();
	}

	void Renderer::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		AC_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2dStorage::CameraData));

		s_Data.QuadRenderer->Begin();
		s_Data.CircleRenderer->Begin();
	}

	void Renderer::EndScene()
	{
		AC_PROFILE_FUNCTION();

		s_Data.CameraUniformBuffer->Bind();
		s_Data.QuadRenderer->End();
		s_Data.CircleRenderer->End();
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

		s_Data.QuadRenderer->Draw(subTexture->GetTexture(), vertices);
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

		s_Data.QuadRenderer->Draw(texture, vertices);
	}

	void Renderer::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade, int entityId)
	{
		AC_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;

		std::array<CircleVertex, quadVertexCount> vertices;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			vertices[i].WorldPosition = transform * s_Data.QuadVertexPositions[i];
			vertices[i].LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
			vertices[i].Color = color;
			vertices[i].Thickness = thickness;
			vertices[i].Fade = fade;
			vertices[i].EntityId = entityId;
		}

		s_Data.CircleRenderer->Draw(vertices);
	}

	//==============================================================================================
	//   Convinience functions
	//==============================================================================================

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

	//================================================================
	//   Renderer Stats
	//================================================================

	uint32_t Renderer::GetDrawCalls()
	{
		return s_Data.QuadRenderer->GetStats().DrawCalls + s_Data.CircleRenderer->GetStats().DrawCalls;
	}

	uint32_t Renderer::GetQuadCount()
	{
		return s_Data.QuadRenderer->GetStats().ObjectCount + s_Data.CircleRenderer->GetStats().ObjectCount;
	}

	uint32_t Renderer::GetIndexCount()
	{
		return s_Data.QuadRenderer->GetStats().GetTotalIndexCount() + s_Data.CircleRenderer->GetStats().GetTotalIndexCount();
	}

	uint32_t Renderer::GetVertexCount()
	{
		return s_Data.QuadRenderer->GetStats().GetTotalVertexCount() + s_Data.CircleRenderer->GetStats().GetTotalVertexCount();
	}

	void Renderer::ResetStats()
	{
		s_Data.QuadRenderer->ResetStats();
		s_Data.CircleRenderer->ResetStats();
	}
}