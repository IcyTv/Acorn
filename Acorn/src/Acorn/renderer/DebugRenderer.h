#pragma once

#include "core/Core.h"

#include "EditorCamera.h"
#include "ecs/components/Components.h"

namespace Acorn
{
	namespace debug
	{

		enum class GizmoType
		{
			Camera,
		};

		class Renderer
		{
		public:
			static void Init();
			static void ShutDown();

			static void Begin(const EditorCamera& camera);
			static void End();

			static void DrawGizmo(GizmoType type, const glm::vec3& position, int entityId = -1, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, const glm::vec2& scale = {1.0f, 1.0f});
			static void DrawCameraFrustum(const Components::CameraComponent& camera, const Components::Transform& transform);
			static void DrawB2dCollider(const Components::BoxCollider2d& collider, const Components::Transform& transform);
			static void DrawB2dCollider(const Components::CircleCollider2d& collider, const Components::Transform& transform);

			static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f});

			//Stats
			static uint32_t GetDrawCalls();
			static uint32_t GetQuadCount();
			static uint32_t GetVertexCount();
			static uint32_t GetIndexCount();
			static void ResetStats();

		private:
		};
	}
}