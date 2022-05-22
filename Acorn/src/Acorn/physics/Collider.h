#pragma once

#include "core/Core.h"
// #include "ecs/Entity.h"

#include <glm/glm.hpp>

namespace Acorn
{
	class Entity;
	namespace Components
	{
		struct Transform;
	}

	namespace Physics2D
	{
		class Collider
		{
		public:
			Collider() = default;
			Collider(const Collider&) = default;
			virtual ~Collider() = default;

			virtual bool IsInside(const glm::vec2& point) = 0;
			virtual void CreateFixture(Entity entity, const Components::Transform& transform) = 0;

			const glm::vec2& GetOffset() const { return m_Offset; }
			glm::vec2& GetOffset() { return m_Offset; }
			void SetOffset(const glm::vec2& offset) { m_Offset = offset; }

		protected:
			Collider(const glm::vec2 offset)
				: m_Offset(offset) {}

			glm::vec2 m_Offset = glm::vec2{0.0f};

			void* m_RuntimeFixture;
		};

		class BoxCollider : public Collider
		{
		public:
			BoxCollider() = default;
			BoxCollider(const BoxCollider&) = default;
			BoxCollider(const glm::vec2& offset, const glm::vec2& size);
			~BoxCollider() {}

			virtual bool IsInside(const glm::vec2& point) override;
			virtual void CreateFixture(Entity entity, const Components::Transform& transform) override;

			const glm::vec2& GetSize() const { return m_Size; }
			glm::vec2& GetSize() { return m_Size; }
			void SetSize(const glm::vec2& size) { m_Size = size; }

		private:
			glm::vec2 m_Size = glm::vec2{1.0f};
		};

		class CircleCollider : public Collider
		{
		public:
			CircleCollider() = default;
			CircleCollider(const CircleCollider&) = default;
			CircleCollider(const glm::vec2& offset, float radius);
			~CircleCollider() {}

			virtual bool IsInside(const glm::vec2& point) override;
			virtual void CreateFixture(Entity entity, const Components::Transform& transform) override;

			float GetRadius() const { return m_Radius; }
			float& GetRadius() { return m_Radius; }
			void SetRadius(float radius) { m_Radius = radius; }

		private:
			float m_Radius = 0.5f;
		};

	}
}