#include "acpch.h"

#include "ecs/components/Components.h"

namespace Acorn::Components
{
	void ChildRelationship::AddEntity(Entity parent, Entity child, Ref<Scene> scene)
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_INFO("Adding child relationship {} -> {}", parent.GetComponent<Tag>().TagName, child.GetComponent<Tag>().TagName);
		Entities.push_back(child);
		if (child.HasComponent<ParentRelationship>())
		{
			child.GetComponent<ParentRelationship>().Parent.GetComponent<Components::ChildRelationship>().RemoveEntity(child);
		}
		child.AddComponent<ParentRelationship>(parent);
	}

	void ChildRelationship::RemoveEntity(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		Entities.erase(std::remove(Entities.begin(), Entities.end(), entity), Entities.end());
		entity.RemoveComponent<ParentRelationship>();
	}

	void ChildRelationship::Clear()
	{
		AC_PROFILE_FUNCTION();
		for (Entity e : Entities)
		{
			e.RemoveComponent<ParentRelationship>();
		}
		Entities.clear();
	}

	bool ChildRelationship::Contains(const UUID& uuid)
	{
		AC_PROFILE_FUNCTION();
		for (auto& entity : Entities)
		{
			if (entity.GetUUID() == uuid)
				return true;
		}
		return false;
	}

	bool ChildRelationship::Contains(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		return std::find(Entities.begin(), Entities.end(), entity) != Entities.end();
	}

	bool ChildRelationship::Empty() const
	{
		AC_PROFILE_FUNCTION();
		return Entities.empty();
	}

}