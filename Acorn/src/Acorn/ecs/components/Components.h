#pragma once

#include "SceneCamera.h"
#include "V8Script.h"
#include "core/Core.h"
#include "core/Timestep.h"
#include "core/UUID.h"
#include "renderer/Texture.h"
#include "utils/FileUtils.h"

#include <cereal/types/string.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <magic_enum.hpp>
#include <v8.h>

namespace Acorn
{
	class ScriptableEntity;
	namespace Components
	{
		struct ID
		{
			Acorn::UUID UUID;

			ID() = default;
			ID(const ID&) = default;
			ID(const Acorn::UUID& uuid)
				: UUID(uuid) {}

			template <class Archive>
			void save(Archive& ar) const
			{
				std::string uuid = (std::string)UUID;
				ar(uuid);
			}

			template <class Archive>
			void load(Archive& ar)
			{
				std::string uuid;
				ar(uuid);
				UUID = uuid;
			}
		};

		struct Tag
		{
			std::string TagName;

			Tag() = default;
			Tag(const Tag&) = default;
			Tag(const std::string& tag)
				: TagName(tag) {}

			template <class Archive>
			void serialize(Archive& ar)
			{
				ar(TagName);
			}
		};

		struct Transform
		{
			glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
			///Rotation in Randians
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

			template <class Archive>
			void serialize(Archive& ar)
			{
				ar(
					Translation.x,
					Translation.y,
					Translation.z,
					Rotation.x,
					Rotation.y,
					Rotation.z,
					Scale.x,
					Scale.y,
					Scale.z);
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

			template <class Archive>
			void save(Archive& ar) const
			{
				std::string texture = "";
				if (Texture)
				{
					texture = Texture->GetPath();
				}
				ar(
					texture,
					Color.x,
					Color.y,
					Color.z,
					Color.w,
					TilingFactor);
			}

			template <class Archive>
			void load(Archive& ar)
			{
				std::string texture;
				ar(texture, Color.x, Color.y, Color.z, Color.w, TilingFactor);
				Texture.reset();
				// Texture = Texture2d::Create(1, 1);
				if (texture.length() > 0)
				{
					Texture = Texture2d::Create(texture);
				}
				else
				{
					Texture = nullptr;
				}
			}
		};

		struct CameraComponent
		{
			SceneCamera Camera;
			bool Primary = true; //TODO move to scene
			bool FixedAspectRatio = false;

			CameraComponent() = default;
			CameraComponent(const CameraComponent&) = default;

			template <class Archive>
			void serialize(Archive& ar)
			{
				ar(
					Camera,
					Primary,
					FixedAspectRatio);
			}
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

			// template <class Archive>
			// void serialize(Archive& ar)
			// {
			// 	ar();
			// }
		};

		struct JSScript
		{
			//TODO ref, because V8Engine keeps a reference to the Script as well...?
			//TODO multiple scripts per entity?
			V8Script* Script = nullptr;

			JSScript() = default;
			JSScript(const std::string& filepath)
			{
				Script = new V8Script(filepath);
			}
			~JSScript()
			{
				//TODO memory leak?
				// delete Script;
			}

			void LoadScript(const std::string& filepath)
			{
				Script = new V8Script(filepath);
			}

			void OnUpdate(Timestep ts)
			{
				if (Script)
				{
					Script->OnUpdate(ts);
				}
			}

			template <class Archive>
			void save(Archive& archive) const
			{
				if (Script)
				{
					std::string path = Script->GetFilePath();
					archive(path);
				}
				else
				{
					archive("");
				}
			}

			template <class Archive>
			void load(Archive& archive)
			{
				std::string path;
				archive(path);
				if (path.length() > 0)
				{
					Script = new V8Script(path);
				}
			}

			// template <class Archive>
			// void serialize(Archive& ar)
			// {
			// 	//TODO serialize member variables!
			// 	ar(Path);
			// }
		};

		//Physics
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

			//TODO move into physics material later
			float Density = 1.0f;
			float Friction = 1.0f;
			float Restitution = 0.0f;
			float RestitutionThreshold = 0.5f;

			//Storage for runtime
			//NOTE this might not have to be stored here, but as a map in scene
			void* RuntimeBody = nullptr;

			RigidBody2d() = default;
			RigidBody2d(const RigidBody2d&) = default;

			void AddForce(const glm::vec2& force);

			template <class Archive>
			void save(Archive& ar) const
			{
				int type = *magic_enum::enum_index(Type);
				ar(type,
				   FixedRotation,
				   Density,
				   Friction,
				   Restitution,
				   RestitutionThreshold);
			}

			template <class Archive>
			void load(Archive& ar)
			{
				int type;
				ar(type,
				   FixedRotation,
				   Density,
				   Friction,
				   Restitution,
				   RestitutionThreshold);
				Type = magic_enum::enum_value<BodyType>(type);
			}
		};

		struct BoxCollider2d
		{
			glm::vec2 Offset = {0.0f, 0.0f};
			glm::vec2 Size = {0.5f, 0.5f};

			//Storage for runtime
			//NOTE this might not have to be stored here, but as a map in scene
			void* RuntimeFixture = nullptr;

			BoxCollider2d() = default;
			BoxCollider2d(const BoxCollider2d&) = default;

			template <class Archive>
			void serialize(Archive& ar)
			{
				ar(
					Offset.x,
					Offset.y,
					Size.x,
					Size.y);
			}
		};
	}
}