#pragma once

#include "renderer/Camera.h"
#include "renderer/Texture.h"

#include "SubTexture2d.h"

namespace Acorn::ext2d //Extension 2d
{
	class Renderer
	{
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(const OrthographicCamera &camera); //TODO remove
		static void BeginScene(const Camera &camera, const glm::mat4 &transform);
		static void EndScene();
		static void Flush();

		//==========
		//Primitives
		//==========

		//Quad
		static void FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);
		static void FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
		static void FillQuad(const glm::vec2 &position, const glm::vec2 &size, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3 &position, const glm::vec2 &size, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2 &position, const glm::vec2 &size, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3 &position, const glm::vec2 &size, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &tint, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);

		//Matrix Quad
		static void FillQuad(const glm::mat4 &transform, const glm::vec4 &color);
		static void FillQuad(const glm::mat4 &transform, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::mat4 &transform, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::mat4 &transform, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillQuad(const glm::mat4 &transform, const glm::vec4 &tint, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);

		//Rotated Quad

		static void FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);
		static void FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);
		static void FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<Texture2d> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);
		static void FillRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &tint, const Ref<SubTexture> &texture, float tilingfactor = 1.0f);

		//Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();

	private:
		static void FlushAndReset();
	};
}