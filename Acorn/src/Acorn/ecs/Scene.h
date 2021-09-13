#pragma once

#include <entt/entt.hpp>

#include "core/Timestep.h"
#include "renderer/EditorCamera.h"

namespace Acorn
{
	class Entity;
	class SceneSerializer;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		template <typename T>
		std::vector<Entity> GetEntitiesWithComponent()
		{
			std::vector<Entity> entities;
			entities.reserve(m_Registry.size());

			for (auto entity : m_Registry.view<T>())
			{
				entities.push_back(Entity{entity, this});
			}

			return entities;
		}

		Entity GetPrimaryCameraEntity();

	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}