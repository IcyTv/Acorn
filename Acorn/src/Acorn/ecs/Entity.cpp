#include "acpch.h"

#include "ecs/Entity.h"

#include "ecs/components/Components.h"

namespace Acorn
{

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

	Entity::Entity(int handle, Scene* scene)
		: m_EntityHandle((entt::entity)handle), m_Scene(scene)
	{
	}

	void Entity::MoveTransform(const glm::mat4& deltaMatrix)
	{
		AC_CORE_ASSERT(m_Scene != nullptr, "Scene is null");
		AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");
		AC_CORE_ASSERT(HasComponent<Components::Transform>(), "Entity does not have component");

		auto& selfTransform = GetComponent<Components::Transform>();
		selfTransform.SetFromMatrix(deltaMatrix * selfTransform.GetTransform());

		if (HasComponent<Components::ChildRelationship>())
		{
			auto& childComponent = GetComponent<Components::ChildRelationship>();

			for (auto& child : childComponent.Entities)
			{
				// FIXME: If the component is a child of another entity, we should rotate the child based on the the parent's center.
				child.MoveTransform(deltaMatrix);
			}
		}
	}

	UUID& Entity::GetUUID()
	{ 
		return GetComponent<Components::ID>().UUID; 
	}

	std::string Entity::GetName()
	{ 
		return GetComponent<Components::Tag>().TagName; 
	}

}