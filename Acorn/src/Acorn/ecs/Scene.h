#pragma once

#include "core/Timestep.h"
#include "renderer/EditorCamera.h"

#include <entt/entity/fwd.hpp>
#include <entt/entity/snapshot.hpp>
#include <entt/entt.hpp>

#include "core/UUID.h"

class b2World;

namespace Acorn
{
	class Entity;
	class SceneSerializer;

	struct SceneOptions
	{
		bool ShowColliders = true;
		bool ShowCameraFrustums = true;
		bool ShowIcons = true;
	};

	class Scene
	{
	public:
		Scene();
		Scene(const Scene& other);
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = "", const UUID& uuid = UUID());
		void DestroyEntity(Entity entity);

		void InitializeRuntime();
		void DestroyRuntime();

		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdateRuntime(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		void RenderFromCamera(Entity entity);

		SceneOptions& GetOptions() { return m_Options; }

		void Snapshot();
		void LoadLastSnapshot();

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

		inline const entt::registry& GetCurrentRegistry() const
		{
			return m_Registry;
		}

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		SceneOptions m_Options;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};
}