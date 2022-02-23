#pragma once

#include "SceneCamera.h"
#include "core/Core.h"
#include "core/Timestep.h"
#include "core/UUID.h"
#include "physics/Collider.h"
#include "renderer/Texture.h"
#include "utils/FileUtils.h"
#ifndef NO_SCRIPTING
	#include "V8Script.h"
	#include <v8.h>
#endif // !NO_SCRIPTING
#include "ecs/Scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <magic_enum.hpp>

namespace Acorn
{
	class ScriptableEntity;
	class Entity;
	namespace Components
	{
		struct ID
		{
			Acorn::UUID UUID;

			ID() = default;
			ID(const ID&) = default;
			ID(const Acorn::UUID& uuid)
				: UUID(uuid) {}
		};

		struct Tag
		{
			std::string TagName;

			Tag() = default;
			Tag(const Tag&) = default;
			Tag(const std::string& tag)
				: TagName(tag) {}
		};

		struct Transform
		{
			glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
			/// Rotation in Randians
			glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
			glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

			Transform() = default;
			Transform(const Transform&) = default;
			Transform(glm::vec3 translation)
				: Translation(translation) {}
			Transform(glm::vec3 translation, glm::vec3 rotation)
				: Translation(translation), Rotation(rotation) {}
			Transform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
				: Translation(translation), Rotation(rotation), Scale(scale) {}

			void SetFromMatrix(const glm::mat4& matrix)
			{
				glm::vec3 scale, translation;
				glm::quat rotation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(matrix, scale, rotation, translation, skew, perspective);

				Translation = translation;
				Rotation = glm::eulerAngles(rotation);
				Scale = scale;
			}

			glm::mat4 GetTransform() const
			{
				auto rotation = glm::toMat4(glm::quat(Rotation));

				auto transform = glm::translate(glm::mat4(1.0f), Translation) *
								 rotation *
								 glm::scale(glm::mat4(1.0f), Scale);
				return transform;
			}

			operator glm::mat4() const
			{
				auto rotation = glm::toMat4(glm::quat(Rotation));

				auto transform = glm::translate(glm::mat4(1.0f), Translation) *
								 rotation *
								 glm::scale(glm::mat4(1.0f), Scale);
				return transform;
			}
		};

		struct SpriteRenderer
		{
			glm::vec4 Color{1.0f};
			Ref<Texture2d> Texture;
			float TilingFactor = 1.0f;

			SpriteRenderer() = default;
			SpriteRenderer(const SpriteRenderer&) = default;
			SpriteRenderer(glm::vec4 color)
				: Color(color) {}
		};

		struct CircleRenderer
		{
			glm::vec4 Color{1.0f};
			// float Radius = 0.5f;
			float Thickness = 1.0f;
			float Fade = 0.005f;

			CircleRenderer() = default;
			CircleRenderer(const CircleRenderer&) = default;
			CircleRenderer(glm::vec4 color)
				: Color(color) {}
		};

		struct CameraComponent
		{
			SceneCamera Camera;
			bool Primary = true; // TODO move to scene
			bool FixedAspectRatio = false;

			CameraComponent() = default;
			CameraComponent(const CameraComponent&) = default;
		};
		struct NativeScript
		{
			ScriptableEntity* Instance = nullptr;

			ScriptableEntity* (*InstantiateScript)();
			void (*DestroyScript)(NativeScript*);

			template <typename T>
			void Bind()
			{
				InstantiateScript = []()
				{
					return static_cast<ScriptableEntity*>(new T());
				};
				DestroyScript = [](NativeScript* script)
				{
					delete script->Instance;
					script->Instance = nullptr;
				};
			}
		};

#ifndef NO_SCRIPTING
		struct JSScript
		{
			// TODO ref, because V8Engine keeps a reference to the Script as well...?
			// TODO multiple scripts per entity?
			V8Script* Script = nullptr;
			bool Watching = false;

			JSScript() = default;
			JSScript(const std::string& filepath)
			{
				Script = new V8Script(filepath);
			}
			~JSScript()
			{
				// TODO memory leak?
				//  delete Script;
			}

			void LoadScript(const std::string& filepath)
			{
				Script = new V8Script(filepath);
				if (Watching)
					Script->Watch();
			}

			void Watch()
			{
				if (Script)
				{
					Script->Watch();
				}
				Watching = true;
			}

			void OnUpdate(Timestep ts)
			{
				if (Script)
				{
					Script->OnUpdate(ts);
				}
			}
		};

#else
		struct JSScript
		{
			bool Watching = false;
			JSScript() = default;
			JSScript(const std::string& filepath) {}
			~JSScript() {}

			void LoadScript(const std::string& filepath) {}
			void Watch() {}
			void OnUpdate(Timestep ts) {}
		};

#endif // !NO_SCRIPT

		// Physics
		struct RigidBody2d
		{
			enum class BodyType
			{
				Static = 0,
				Dynamic,
				Kinematic,
			};
			BodyType Type = BodyType::Static;
			bool FixedRotation = false;

			// TODO move into physics material later
			float Density = 1.0f;
			float Friction = 1.0f;
			float Restitution = 0.0f;
			float RestitutionThreshold = 0.5f;

			// Storage for runtime
			// NOTE this might not have to be stored here, but as a map in scene
			void* RuntimeBody = nullptr;

			RigidBody2d() = default;
			RigidBody2d(const RigidBody2d&) = default;

			void AddForce(const glm::vec2& force);
		};

		// struct BoxCollider2d
		// {
		// 	glm::vec2 Offset = {0.0f, 0.0f};
		// 	glm::vec2 Size = {0.5f, 0.5f};

		// 	//Storage for runtime
		// 	//NOTE this might not have to be stored here, but as a map in scene
		// 	void* RuntimeFixture = nullptr;

		// 	BoxCollider2d() = default;
		// 	BoxCollider2d(const BoxCollider2d&) = default;
		// };

		// struct CircleCollider2d
		// {
		// 	glm::vec2 Offset = {0.0f, 0.0f};
		// 	float Radius = 0.5f;

		// 	//Storage for runtime
		// 	void* RuntimeFixture = nullptr;

		// 	CircleCollider2d() = default;
		// 	CircleCollider2d(const CircleCollider2d&) = default;
		// };

		typedef Physics2D::BoxCollider BoxCollider2d;
		typedef Physics2D::CircleCollider CircleCollider2d;

		struct ParentRelationship
		{
			int Parent = -1; // TODO pointer is invalid

			ParentRelationship() = default;
			ParentRelationship(const ParentRelationship&) = default;

			ParentRelationship(int parent)
				: Parent(parent) {}
		};

		struct ChildRelationship
		{
			std::vector<Entity> Entities;

			ChildRelationship() = default;
			ChildRelationship(const ChildRelationship&) = default;

			void AddEntity(Entity parent, Entity child, Ref<Scene> scene);
			void RemoveEntity(Entity entity);
			void Clear();

			bool Empty() const;
			bool Contains(Entity entity);
			bool Contains(const UUID& uuid);
		};
	}
}