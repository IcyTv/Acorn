#include "acpch.h"

#include "ecs/Entity.h"

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

}