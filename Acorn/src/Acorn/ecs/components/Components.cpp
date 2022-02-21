#include "acpch.h"

#include "ecs/Entity.h"
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
			Entity origParent{child.GetComponent<ParentRelationship>().Parent, scene.get()};
			origParent.GetComponent<Components::ChildRelationship>().RemoveEntity(child);
			// child.GetComponent<ParentRelationship>().Parent = (int)(uint32_t)parent;
		}
		child.AddComponent<ParentRelationship>((int)(uint32_t)parent);
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