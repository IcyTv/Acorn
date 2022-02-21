#include "acpch.h"

#include "ecs/Entity.h"
#include "ecs/components/Components.h"
#include "physics/Collider.h"

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_shape.h>

namespace Acorn
{
	namespace Physics2D
	{
		BoxCollider::BoxCollider(const glm::vec2& size, const glm::vec2& offset)
			: Collider(offset), m_Size(size)
		{
		}

		bool BoxCollider::IsInside(const glm::vec2& point)
		{
			AC_PROFILE_FUNCTION();
			b2Fixture* fixture = static_cast<b2Fixture*>(m_RuntimeFixture);
			b2Shape* shape = static_cast<b2Shape*>(fixture->GetShape());
			return shape->TestPoint(fixture->GetBody()->GetTransform(), b2Vec2(point.x, point.y));
		}

		void BoxCollider::CreateFixture(Entity entity, const Components::Transform& transform)
		{
			AC_PROFILE_FUNCTION();
			AC_CORE_ASSERT(entity.HasComponent<Components::RigidBody2d>(), "Entity does not have a RigidBody2d component!");

			Components::RigidBody2d rigidBody = entity.GetComponent<Components::RigidBody2d>();

			b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);

			b2PolygonShape shape;
			shape.SetAsBox(m_Size.x * transform.Scale.x, m_Size.y * transform.Scale.y, b2Vec2{m_Offset.x, m_Offset.y}, 0.0f);

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &shape;
			fixtureDef.density = rigidBody.Density;
			fixtureDef.friction = rigidBody.Friction;
			fixtureDef.restitution = rigidBody.Restitution;
			fixtureDef.restitutionThreshold = rigidBody.RestitutionThreshold;

			m_RuntimeFixture = body->CreateFixture(&fixtureDef);
		}

		CircleCollider::CircleCollider(const glm::vec2& offset, float radius)
			: Collider(offset), m_Radius(radius)
		{
		}

		bool CircleCollider::IsInside(const glm::vec2& point)
		{
			AC_PROFILE_FUNCTION();
			b2Fixture* fixture = static_cast<b2Fixture*>(m_RuntimeFixture);
			b2Shape* shape = static_cast<b2Shape*>(fixture->GetShape());
			return shape->TestPoint(fixture->GetBody()->GetTransform(), b2Vec2(point.x, point.y));
		}

		void CircleCollider::CreateFixture(Entity entity, const Components::Transform& transform)
		{
			AC_PROFILE_FUNCTION();
			AC_CORE_ASSERT(entity.HasComponent<Components::RigidBody2d>(), "Entity does not have a RigidBody2d component!");

			Components::RigidBody2d rigidBody = entity.GetComponent<Components::RigidBody2d>();

			b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);

			b2CircleShape shape;
			shape.m_p = b2Vec2{m_Offset.x, m_Offset.y};
			shape.m_radius = m_Radius;

			b2FixtureDef fixtureDef;
			fixtureDef.shape = &shape;
			fixtureDef.density = rigidBody.Density;
			fixtureDef.friction = rigidBody.Friction;
			fixtureDef.restitution = rigidBody.Restitution;
			fixtureDef.restitutionThreshold = rigidBody.RestitutionThreshold;

			m_RuntimeFixture = body->CreateFixture(&fixtureDef);
		}
	}
}