#pragma once

#include "renderer/Camera.h"
#include "renderer/EditorCamera.h"
#include "renderer/Texture.h"

#include "ecs/components/Components.h"

#include "SubTexture2d.h"

namespace Acorn::ext2d //Extension 2d
{
	class Renderer
	{
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();

		//==========
		//Primitives
		//==========

		//Quad
		static void FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void FillQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& tint, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);

		//Matrix Quad
		static void FillQuad(const glm::mat4& transform, const glm::vec4& color);
		static void FillQuad(const glm::mat4& transform, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::mat4& transform, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor = 1.0f, int entityID = -1);
		static void FillQuad(const glm::mat4& transform, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::mat4& transform, const glm::vec4& tint, const Ref<SubTexture>& texture, float tilingfactor = 1.0f, int entityID = -1);

		//Rotated Quad

		static void FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<Texture2d>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& tint, const Ref<SubTexture>& texture, float tilingfactor = 1.0f);

		//Sprites
		static void DrawSprite(const glm::mat4& transform, Components::SpriteRenderer& sprite, int entityId);

		//Circles
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityId = -1);

		//Stats
		//FIXME the reason these exist, is because I don't want to expose a header implemented BatchRenderer... Maybe there is a way to implement it in a cpp file?...
		static uint32_t GetDrawCalls();
		static uint32_t GetQuadCount();
		static uint32_t GetVertexCount();
		static uint32_t GetIndexCount();
		static void ResetStats();

	private:
	};
}