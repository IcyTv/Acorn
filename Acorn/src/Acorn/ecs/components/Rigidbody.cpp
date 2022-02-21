#include "ecs/components/Components.h"

#include <box2d/b2_body.h>

namespace Acorn::Components
{
	void RigidBody2d::AddForce(const glm::vec2& force)
	{
		if (!RuntimeBody)
		{
			AC_CORE_WARN("Tried applying a force to a non-initialized body!");
			return;
		}

		AC_CORE_ASSERT(Type == BodyType::Kinematic || Type == BodyType::Dynamic, "Tried applying a force to a static body!");

		b2Body* body = static_cast<b2Body*>(RuntimeBody);
		body->ApplyForce(b2Vec2(force.x, force.y), body->GetWorldCenter(), true);
	}
}