#include "acpch.h"

#include "Scene.h"

#include "Entity.h"
#include "components/Components.h"
#include "ecs/components/Components.h"
#include "ecs/components/V8Script.h"
#include "renderer/2d/Renderer2D.h"
#include "renderer/DebugRenderer.h"

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

#include <glm/glm.hpp>

namespace Acorn
{
	static b2BodyType GetBodyType(Components::RigidBody2d::BodyType type)
	{
		switch (type)
		{
			case Components::RigidBody2d::BodyType::Static:
				return b2_staticBody;
			case Components::RigidBody2d::BodyType::Kinematic:
				return b2_kinematicBody;
			case Components::RigidBody2d::BodyType::Dynamic:
				return b2_dynamicBody;
		}
		AC_CORE_ASSERT(false, "Invalid box2d body type");
		return b2_staticBody;
	}

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

	void Scene::InitializeRuntime()
	{
		m_PhysicsWorld = new b2World(b2Vec2(0.0f, -9.8f));

		auto rigidBodies = m_Registry.view<Components::RigidBody2d>();
		for (auto e : rigidBodies)
		{
			Entity entity = {e, this};
			//TODO check if using the registry is faster
			auto& transform = entity.GetComponent<Components::Transform>();
			auto& rigidBody = entity.GetComponent<Components::RigidBody2d>();

			b2BodyDef bodyDef;
			bodyDef.type = GetBodyType(rigidBody.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rigidBody.FixedRotation);

			rigidBody.RuntimeBody = body;

			if (entity.HasComponent<Components::BoxCollider2d>())
			{
				auto& collider = entity.GetComponent<Components::BoxCollider2d>();

				b2PolygonShape shape;
				shape.SetAsBox(collider.Size.x * transform.Scale.x, collider.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = collider.Density;
				fixtureDef.friction = collider.Friction;
				fixtureDef.restitution = collider.Restitution;
				fixtureDef.restitutionThreshold = collider.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		}

		//Setup v8
		V8Engine::instance().KeepRunning();
		auto view = m_Registry.view<Components::JSScript>();
		for (auto entity : view)
		{
			auto& script = view.get<Components::JSScript>(entity);
			if (script.Script != nullptr)
			{
				script.Script->Load({entity, this});
			}
			else
			{
				auto& tag = m_Registry.get<Components::Tag>(entity).TagName;
				AC_CORE_WARN("Undefined script in {}", tag);
			}
		}
	}

	void Scene::DestroyRuntime()
	{
		auto view = m_Registry.view<Components::JSScript>();
		for (auto entity : view)
		{
			auto& script = view.get<Components::JSScript>(entity);
			if (script.Script != nullptr)
			{
				script.Script->Dispose();
			}
		}
		//TODO
		// V8Engine::instance().Stop();

		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
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

		// {
		// 	//Draw Icon Gizmos
		// 	debug::Renderer::Begin(camera);

		// 	auto group = m_Registry.group<Components::CameraComponent>(entt::get<Components::Transform>);
		// 	for (auto&& [entity, camera, transform] : group.each())
		// 	{
		// 		debug::Renderer::DrawGizmo(debug::GizmoType::Camera, transform.Translation, (int)entity);
		// 	}

		// 	debug::Renderer::End();
		// }
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
		{
			m_Registry.view<Components::JSScript>().each(
				[=](auto entity, Components::JSScript& jsc)
				{
					if (jsc.Script != nullptr)
					{
						jsc.Script->OnUpdate(ts);
					}
				});
		}

		//Physics
		{
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;

			m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

			auto view = m_Registry.view<Components::RigidBody2d>();
			for (auto e : view)
			{
				Entity entity = {e, this};
				auto& rigidBody = entity.GetComponent<Components::RigidBody2d>();
				auto& transform = entity.GetComponent<Components::Transform>();

				b2Body* body = static_cast<b2Body*>(rigidBody.RuntimeBody);
				b2Vec2 position = body->GetPosition();

				transform.Translation.x = position.x;
				transform.Translation.y = position.y;

				transform.Rotation.z = body->GetAngle();
			}
		}

		//Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<Components::Transform, Components::CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<Components::Transform, Components::CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
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
		else
		{
			AC_CORE_WARN("No camera found in scene!");
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

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::JSScript& component)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::RigidBody2d& component)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::BoxCollider2d& component)
	{
	}
}