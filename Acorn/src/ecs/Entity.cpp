#include "acpch.h"

#include "Entity.h"

namespace Acorn
{

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{

	}

}