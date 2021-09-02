#include "acpch.h"

#include "Scene.h"

#include "Entity.h"
#include "components/Components.h"
#include "renderer/2d/Renderer2D.h"
#include "renderer/DebugRenderer.h"

#include <glm/glm.hpp>

namespace Acorn
{

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = {m_Registry.create(), this};
		entity.AddComponent<Components::Transform>();
		auto& tag = entity.AddComponent<Components::Tag>();
		tag.TagName = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		ext2d::Renderer::BeginScene(camera);
		auto group = m_Registry.group<Components::Transform>(entt::get<Components::SpriteRenderer>);
		for (auto&& [entity, transform, sprite] : group.each())
		{
			// ext2d::Renderer::FillQuad((glm::mat4)transform, sprite.Color);
			ext2d::Renderer::DrawSprite((glm::mat4)transform, sprite, (int)entity);
		}

		ext2d::Renderer::EndScene();

		{
			//Draw Icon Gizmos
			debug::Renderer::Begin(camera);

			auto group = m_Registry.group<Components::CameraComponent>(entt::get<Components::Transform>);
			for (auto&& [entity, camera, transform] : group.each())
			{
				debug::Renderer::DrawGizmo(debug::GizmoType::Camera, transform.Translation, (int)entity);
			}

			debug::Renderer::End();
		}
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		//Update Scripts
		{
			m_Registry.view<Components::NativeScript>().each(
				[=](auto entity, Components::NativeScript& nsc)
				{
					//TODO: Move to Scene::OnScenePlay
					if (nsc.Instance == nullptr)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{entity, this};
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
				});
		}
		//Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<Components::CameraComponent, Components::Transform>();

			for (auto entity : view)
			{
				auto& camera = view.get<Components::CameraComponent>(entity);
				auto& transform = view.get<Components::Transform>(entity);
				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = (glm::mat4)transform;
					break;
				}
			}
		}

		if (mainCamera)
		{
			ext2d::Renderer::BeginScene(*mainCamera, cameraTransform);
			auto group = m_Registry.group<Components::Transform>(entt::get<Components::SpriteRenderer>);
			for (auto&& [entity, transform, sprite] : group.each())
			{
				ext2d::Renderer::DrawSprite((glm::mat4)transform, sprite, (int)entity);
			}

			ext2d::Renderer::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<Components::CameraComponent>();
		for (auto&& [entity, camera] : view.each())
		{
			if (!camera.FixedAspectRatio)
			{
				if (width > 0 && height > 0)
					camera.Camera.SetViewportSize(width, height);
			}
		}
	}

	Entity Scene::GetPrimaryCameraEntity()
	{

		auto view = m_Registry.view<Components::CameraComponent>();
		for (auto&& [entity, camera] : view.each())
		{
			if (camera.Primary)
			{
				return Entity{entity, this};
			}
		}

		return {};
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false, "Component not supported");
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::Transform& transform)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::CameraComponent& camera)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		else
			camera.Camera.SetViewportSize(16, 9);
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::SpriteRenderer& sprite)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::NativeScript& nsc)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::Tag& component)
	{
	}
}