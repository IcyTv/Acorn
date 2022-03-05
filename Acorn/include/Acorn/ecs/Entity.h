#pragma once

#include "core/Core.h"

#include "Scene.h"
#include "core/UUID.h"
// #include "components/Components.h"

#include <entt/entt.hpp>

namespace Acorn
{

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(int handle, Scene* scene);
		Entity(const Entity& other) = default;

		template <typename T>
		bool HasComponent()
		{
			AC_CORE_ASSERT(m_Scene != nullptr, "Scene is null");
			AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}

		template <typename... T>
		bool HasComponents()
		{
			AC_CORE_ASSERT(m_Scene != nullptr, "Scene is null");
			AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

			return m_Scene->m_Registry.all_of<T...>(m_EntityHandle);
		}

		template <typename T>
		T& GetComponent()
		{
			AC_CORE_ASSERT(HasComponents<T>(), "Does not have component");
			AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			AC_CORE_ASSERT(!HasComponents<T>(), "Entity already has component");
			AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);

			m_Scene->OnComponentAdded(*this, component);

			return component;
		}

		template <typename T>
		void RemoveComponent()
		{
			AC_CORE_ASSERT(HasComponent<T>(), "Entity does not have component");
			AC_CORE_ASSERT(m_EntityHandle != entt::null, "Entity is null");

			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		/**
		 * @brief Move the transform of this entity by the given transform.
		 *
		 * @param deltaMatrix
		 *  DeltaMatrix to move the transform by.
		 */
		void MoveTransform(const glm::mat4& deltaMatrix);

		UUID& GetUUID();
		std::string GetName();

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }

		bool IsValid() const { return m_EntityHandle != entt::null && m_Scene != nullptr; }

	private:
		entt::entity m_EntityHandle{entt::null};
		Scene* m_Scene = nullptr;
	};
}

namespace std
{
	template <>
	struct hash<Acorn::Entity>
	{
		size_t operator()(const Acorn::Entity& entity) const
		{
			return (size_t)(entt::entity)entity;
		}
	};
}