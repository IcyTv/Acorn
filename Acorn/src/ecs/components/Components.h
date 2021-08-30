#pragma once

#include <glm/glm.hpp>

#include "SceneCamera.h"
#include "core/Core.h"

#include "ScriptableEntity.h"
#include "core/Timestep.h"

namespace Acorn::Components
{
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
		///Rotation in Randians
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(glm::vec3 translation)
			: Translation(translation) {}

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

		SpriteRenderer() = default;
		SpriteRenderer(const SpriteRenderer&) = default;
		SpriteRenderer(glm::vec4 color)
			: Color(color) {}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; //TODO move to scene
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
}