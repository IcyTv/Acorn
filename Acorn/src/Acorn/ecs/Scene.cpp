#include "acpch.h"

#include "Scene.h"
#include "ecs/components/ScriptableEntity.h"

#include "Entity.h"
#include "components/Components.h"
#include "ecs/components/V8Script.h"
#include "renderer/2d/Renderer2D.h"
#include "renderer/DebugRenderer.h"

#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_collision.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

#include <glm/matrix.hpp>
#include <magic_enum.hpp>

#include <entt/entity/fwd.hpp>
#include <entt/entity/helper.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entity/snapshot.hpp>
#include <glm/glm.hpp>

namespace Acorn
{
	static b2BodyType GetBodyType(Components::RigidBody2d::BodyType type)
	{
		AC_PROFILE_FUNCTION();
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

	template <typename Component>
	static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& entityMap)
	{
		AC_PROFILE_FUNCTION();
		auto view = src.view<Component>();
		for (auto entity : view)
		{
			UUID uuid = src.get<Components::ID>(entity).UUID;
			AC_CORE_ASSERT(entityMap.find(uuid) != entityMap.end(), "Entity not found in entity map");
			entt::entity enttId = entityMap.at(uuid);

			auto& component = src.get<Component>(entity);
			dst.emplace_or_replace<Component>(enttId, component);
		}
	}

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Ref<Scene> Scene::Copy(Ref<Scene> src)
	{
		AC_PROFILE_FUNCTION();
		auto dst = CreateRef<Scene>();

		dst->m_ViewportWidth = src->m_ViewportWidth;
		dst->m_ViewportHeight = src->m_ViewportHeight;

		auto& srcSceneReg = src->m_Registry;
		auto& dstSceneReg = dst->m_Registry;
		auto idView = srcSceneReg.view<Components::ID>();

		std::unordered_map<UUID, entt::entity> entityMap;

		for (auto e : idView)
		{
			Entity entity{e, src.get()};
			UUID uuid = entity.GetUUID();
			const auto& name = entity.GetComponent<Components::Tag>().TagName;

			entt::entity newEntity = dst->CreateEntity(name, uuid);

			entityMap[uuid] = newEntity;
		}

		CopyComponent<Components::Transform>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::SpriteRenderer>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::CircleRenderer>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::CameraComponent>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::NativeScript>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::JSScript>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::RigidBody2d>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::BoxCollider2d>(dstSceneReg, srcSceneReg, entityMap);
		CopyComponent<Components::CircleCollider2d>(dstSceneReg, srcSceneReg, entityMap);

		return dst;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		name += " - Copy";

		Entity newEntity = CreateEntity(name);

		auto& newTransform = newEntity.GetComponent<Components::Transform>();
		auto oldTransform = entity.GetComponent<Components::Transform>();
		newTransform.Translation = oldTransform.Translation;
		newTransform.Rotation = oldTransform.Rotation;
		newTransform.Scale = oldTransform.Scale;

		if (entity.HasComponent<Components::SpriteRenderer>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::SpriteRenderer>(entity.GetComponent<Components::SpriteRenderer>());
		}
		if (entity.HasComponent<Components::CircleRenderer>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::CircleRenderer>(entity.GetComponent<Components::CircleRenderer>());
		}

		if (entity.HasComponent<Components::CameraComponent>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::CameraComponent>(entity.GetComponent<Components::CameraComponent>());
		}

		if (entity.HasComponent<Components::NativeScript>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::NativeScript>(entity.GetComponent<Components::NativeScript>());
		}

		if (entity.HasComponent<Components::JSScript>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::JSScript>(entity.GetComponent<Components::JSScript>());
		}

		if (entity.HasComponent<Components::RigidBody2d>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::RigidBody2d>(entity.GetComponent<Components::RigidBody2d>());
		}

		if (entity.HasComponent<Components::BoxCollider2d>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::BoxCollider2d>(entity.GetComponent<Components::BoxCollider2d>());
		}

		if (entity.HasComponent<Components::CircleCollider2d>())
		{
			// Call the copy constructor
			newEntity.AddComponent<Components::CircleCollider2d>(entity.GetComponent<Components::CircleCollider2d>());
		}

		// TODO figure out relations

		return newEntity;
	}

	Entity Scene::CreateEntity(const std::string& name, const UUID& uuid)
	{
		AC_PROFILE_FUNCTION();
		Entity entity = {m_Registry.create(), this};
		entity.AddComponent<Components::ID>(uuid);
		entity.AddComponent<Components::Transform>();
		auto& tag = entity.AddComponent<Components::Tag>();
		tag.TagName = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		m_Registry.destroy(entity);
	}

	Entity Scene::GetEntity(const UUID& uuid)
	{
		AC_PROFILE_FUNCTION();
		auto view = m_Registry.view<Components::ID>();

		for (auto entity : view)
		{
			if (m_Registry.get<Components::ID>(entity).UUID == uuid)
			{
				return Entity{entity, this};
			}
		}

		return Entity{};
	}

	void Scene::InitializeRuntime()
	{
		AC_PROFILE_FUNCTION();
		m_PhysicsWorld = new b2World(b2Vec2(0.0f, -9.8f));

		auto rigidBodies = m_Registry.view<Components::RigidBody2d>();
		for (auto e : rigidBodies)
		{
			Entity entity = {e, this};
			// TODO check if using the registry is faster
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
				auto& boxCollider = entity.GetComponent<Components::BoxCollider2d>();
				boxCollider.CreateFixture(entity, transform);
				// auto& collider = entity.GetComponent<Components::BoxCollider2d>();

				// b2PolygonShape shape;
				// shape.SetAsBox(collider.Size.x * transform.Scale.x, collider.Size.y * transform.Scale.y, b2Vec2{collider.Offset.x, collider.Offset.y}, 0.0f);

				// b2FixtureDef fixtureDef;
				// fixtureDef.shape = &shape;
				// fixtureDef.density = rigidBody.Density;
				// fixtureDef.friction = rigidBody.Friction;
				// fixtureDef.restitution = rigidBody.Restitution;
				// fixtureDef.restitutionThreshold = rigidBody.RestitutionThreshold;

				// collider.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}
			if (entity.HasComponent<Components::CircleCollider2d>())
			{
				auto& collider = entity.GetComponent<Components::CircleCollider2d>();
				collider.CreateFixture(entity, transform);

				// b2CircleShape shape;
				// shape.m_p.Set(collider.Offset.x, collider.Offset.y);
				// shape.m_radius = collider.Radius * transform.Scale.z; //TODO think about scaling axis

				// b2FixtureDef fixtureDef;
				// fixtureDef.shape = &shape;
				// fixtureDef.density = rigidBody.Density;
				// fixtureDef.friction = rigidBody.Friction;
				// fixtureDef.restitution = rigidBody.Restitution;
				// fixtureDef.restitutionThreshold = rigidBody.RestitutionThreshold;

				// collider.RuntimeFixture = body->CreateFixture(&fixtureDef);
			}
		}

		// Setup v8
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
		AC_PROFILE_FUNCTION();
		auto view = m_Registry.view<Components::JSScript>();
		for (auto entity : view)
		{
			auto& script = view.get<Components::JSScript>(entity);
			if (script.Script != nullptr)
			{
				script.Script->Dispose();
			}
		}
		// TODO
		//  V8Engine::instance().Stop();

		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		AC_PROFILE_FUNCTION();
		ext2d::Renderer::BeginScene(camera);
		{
			auto group = m_Registry.group<Components::Transform>(entt::get<Components::SpriteRenderer>);

			for (auto&& [entity, transform, sprite] : group.each())
			{
				ext2d::Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}

		{
			auto view = m_Registry.view<Components::Transform, Components::CircleRenderer>();
			for (auto&& [entity, transform, circle] : view.each())
			{
				ext2d::Renderer::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
			}
		}

		ext2d::Renderer::EndScene();

		{
			AC_PROFILE_SCOPE("Scene::OnEditorUpdate (DebugRendering)");
			debug::Renderer::Begin(camera);

			auto cameraGroup = m_Registry.group<Components::CameraComponent>(entt::get<Components::Transform>);
			for (auto&& [entity, camera, transform] : cameraGroup.each())
			{
				if (m_Options.ShowIcons)
					debug::Renderer::DrawGizmo(debug::GizmoType::Camera, transform.Translation, (int)entity);
				if (m_Options.ShowCameraFrustums)
					debug::Renderer::DrawCameraFrustum(camera, transform);
			}

			auto b2dColliderGroup = m_Registry.group<Components::BoxCollider2d>(entt::get<Components::Transform>);
			for (auto&& [entity, collider, transform] : b2dColliderGroup.each())
			{
				// TODO Think about, if we even need zDepth for colliders?
				if (m_Options.ShowColliders)
					debug::Renderer::DrawB2dCollider(collider, transform);
			}

			auto b2dCircleColliderGroup = m_Registry.group<Components::CircleCollider2d>(entt::get<Components::Transform>);
			for (auto&& [entity, collider, transform] : b2dCircleColliderGroup.each())
			{
				if (m_Options.ShowColliders)
					debug::Renderer::DrawB2dCollider(collider, transform);
			}

			// TODO show circle colliders

			debug::Renderer::End();
		}
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		AC_PROFILE_FUNCTION();

		// Render 2D
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

		// Update Scripts
		{
			m_Registry.view<Components::NativeScript>().each(
				[=](auto entity, Components::NativeScript& nsc)
				{
					// TODO: Move to Scene::OnScenePlay
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
			V8Data& data = V8Engine::instance().GetData();
			data.PrimaryCameraViewProjectionMatrix = mainCamera->GetProjection() * glm::inverse(cameraTransform);

			m_Registry.view<Components::JSScript>().each(
				[=](auto entity, Components::JSScript& jsc)
				{
					if (jsc.Script != nullptr)
					{
						jsc.Script->OnUpdate(ts);
					}
				});
		}

		// Physics
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

		if (mainCamera)
		{
			ext2d::Renderer::BeginScene(*mainCamera, cameraTransform);

			{
				auto group = m_Registry.group<Components::Transform>(entt::get<Components::SpriteRenderer>);
				for (auto&& [entity, transform, sprite] : group.each())
				{
					ext2d::Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}

			{
				auto view = m_Registry.view<Components::Transform, Components::CircleRenderer>();
				for (auto&& [entity, transform, circle] : view.each())
				{
					ext2d::Renderer::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
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
		AC_PROFILE_FUNCTION();
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

	void Scene::RenderFromCamera(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_ASSERT(entity.HasComponent<Components::CameraComponent>(), "Entity does not have a camera component");
		auto& camera = entity.GetComponent<Components::CameraComponent>();
		auto& transform = entity.GetComponent<Components::Transform>();

		ext2d::Renderer::BeginScene(camera.Camera, transform.GetTransform());

		auto group = m_Registry.group<Components::Transform>(entt::get<Components::SpriteRenderer>);
		for (auto&& [entity, transform, sprite] : group.each())
		{
			ext2d::Renderer::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		}

		ext2d::Renderer::EndScene();
	}

	void Scene::Snapshot()
	{
	}

	void Scene::LoadLastSnapshot()
	{
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		AC_PROFILE_FUNCTION();

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

	// TODO remove template specialization
	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T&)
	{
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
	void Scene::OnComponentAdded(Entity entity, Components::CircleRenderer& sprite)
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

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::CircleCollider2d& component)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::ParentRelationship& component)
	{
	}

	template <>
	void Scene::OnComponentAdded(Entity entity, Components::ChildRelationship& component)
	{
	}
}