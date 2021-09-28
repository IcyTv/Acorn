#include "acpch.h"

#include "Serializer.h"

#include "core/Core.h"
#include "ecs/Entity.h"
#include "ecs/Scene.h"
#include "ecs/components/Components.h"

#include <filesystem>
#include <fstream>
#include <magic_enum.hpp>
#include <sstream>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

//TODO integrate with cereal

namespace YAML
{
	template <>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Acorn
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::Tag& tagComponent)
	{
		out << YAML::Value << YAML::BeginMap; //TagComponent

		out << YAML::Key << "Tag" << YAML::Value << tagComponent.TagName;

		out << YAML::EndMap; //TagComponent

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::Transform& transformComponent)
	{
		out << YAML::Value << YAML::BeginMap; //TransformComponent

		out << YAML::Key << "Translation" << YAML::Value << transformComponent.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << transformComponent.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << transformComponent.Scale;

		out << YAML::EndMap; //TransformComponent

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::CameraComponent& camera)
	{
		out << YAML::Value << YAML::BeginMap; //CameraComponent

		out << YAML::Key << "SceneCamera" << YAML::BeginMap; //SceneCamera
		out << YAML::Key << "Type" << YAML::Value << (int)camera.Camera.GetProjectionType();
		out << YAML::Key << "OrthographicSize" << YAML::Value << camera.Camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNearClip" << YAML::Value << camera.Camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFarClip" << YAML::Value << camera.Camera.GetOrthographicFarClip();
		out << YAML::Key << "PerspectiveFov" << YAML::Value << camera.Camera.GetPerspectiveFov();
		out << YAML::Key << "PerspectiveNearClip" << YAML::Value << camera.Camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFarClip" << YAML::Value << camera.Camera.GetPerspectiveFarClip();

		out << YAML::EndMap; //SceneCamera

		out << YAML::Key << "FixedAspectRatio" << YAML::Value << camera.FixedAspectRatio;
		out << YAML::Key << "Primary" << YAML::Value << camera.Primary;

		out << YAML::EndMap; //CameraComponent

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::SpriteRenderer& spriteRenderer)
	{
		out << YAML::Value << YAML::BeginMap; //SpriteRenderer

		out << YAML::Key << "Color" << YAML::Value << spriteRenderer.Color;

		out << YAML::EndMap; //SpriteRenderer

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::JSScript& jsScript)
	{
		out << YAML::Value << YAML::BeginMap; //JSScript

		out << YAML::Key << "Path" << YAML::Value << jsScript.Script->GetFilePath();

		if (jsScript.Script)
		{
			out << YAML::Key << "Parameters" << YAML::BeginMap; //User Defined Parameters

			for (auto& param : jsScript.Script->GetParameters())
			{
				out << YAML::Key << param.first << YAML::Value << param.second;
			}

			out << YAML::EndMap; //User Defined Parameters
		}
		out << YAML::EndMap; //JSScript

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::RigidBody2d& rigidBody)
	{
		out << YAML::Value << YAML::BeginMap; //RigidBody2d

		out << YAML::Key << "Type" << YAML::Value << magic_enum::enum_name<Components::RigidBody2d::BodyType>(rigidBody.Type).data();
		out << YAML::Key << "FixedRotation" << YAML::Value << rigidBody.FixedRotation;

		out << YAML::Key << "Density" << YAML::Value << rigidBody.Density;
		out << YAML::Key << "Friction" << YAML::Value << rigidBody.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << rigidBody.Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << rigidBody.RestitutionThreshold;

		out << YAML::EndMap; //RigidBody2d
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Components::BoxCollider2d& boxCollider)
	{
		out << YAML::Value << YAML::BeginMap; //BoxCollider2d

		out << YAML::Key << "Size" << YAML::Value << boxCollider.Size;
		out << YAML::Key << "Offset" << YAML::Value << boxCollider.Offset;

		out << YAML::EndMap; //BoxCollider2d

		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; //Entity

		AC_CORE_ASSERT(entity.HasComponent<Components::ID>(), "Entity does not have an ID, cannot be serialized!");

		out << YAML::Key << "Entity" << YAML::Value << (std::string)entity.GetUUID();

		if (entity.HasComponent<Components::Tag>())
		{
			out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<Components::Tag>();
		}

		if (entity.HasComponent<Components::Transform>())
		{
			out << YAML::Key << "Transform" << YAML::Value << entity.GetComponent<Components::Transform>();
		}

		if (entity.HasComponent<Components::CameraComponent>())
		{
			out << YAML::Key << "Camera" << YAML::Value << entity.GetComponent<Components::CameraComponent>();
		}

		if (entity.HasComponent<Components::SpriteRenderer>())
		{
			out << YAML::Key << "SpriteRenderer" << YAML::Value << entity.GetComponent<Components::SpriteRenderer>();
		}

		if (entity.HasComponent<Components::JSScript>())
		{
			out << YAML::Key << "JSScript" << YAML::Value << entity.GetComponent<Components::JSScript>();
		}

		if (entity.HasComponent<Components::RigidBody2d>())
		{
			out << YAML::Key << "RigidBody2d" << YAML::Value << entity.GetComponent<Components::RigidBody2d>();
		}

		if (entity.HasComponent<Components::BoxCollider2d>())
		{
			out << YAML::Key << "BoxCollider2d" << YAML::Value << entity.GetComponent<Components::BoxCollider2d>();
		}

		out << YAML::EndMap; //Entity
	}

	void SceneSerializer::Serialize(const std::string& filePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each(
			[&](auto entityId)
			{
				Entity entity = {entityId, m_Scene.get()};
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		AC_CORE_TRACE("Writing serialization to {}", filePath);

		if (!std::filesystem::exists(std::filesystem::path(filePath).parent_path()))
		{
			bool success = std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
			AC_CORE_ASSERT(success, "Failed to create directory for serialization file");
		}

		std::ofstream fout(filePath);
		AC_CORE_ASSERT(fout, "Failed to open file for writing!");

		fout << out.c_str();
		fout.close();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filePath)
	{
		AC_CORE_ASSERT(false, "Not implemented");
	}

	bool SceneSerializer::Deserialize(const std::string& filePath)
	{
		//TODO error handling
		std::ifstream fin(filePath);
		std::stringstream buffer;
		buffer << fin.rdbuf();

		YAML::Node root = YAML::Load(buffer.str());
		if (!root["Scene"])
			return false;

		std::string sceneName = root["Scene"].as<std::string>();
		AC_CORE_TRACE("Deserializing scene {}", sceneName);

		auto entities = root["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				// uint64_t uuid = entity["Entity"].as<uint64_t>();
				// uint64_t uuid = 0;
				std::string uuid = entity["Entity"].as<std::string>();

				std::string name;
				auto tagComponent = entity["Tag"];
				if (tagComponent)
				{
					name = tagComponent["Tag"].as<std::string>();
				}

				AC_CORE_TRACE("Deserialized entity [id = {}, name = {}]", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntity(name, uuid);

				auto transformComponent = entity["Transform"];

				if (transformComponent)
				{
					//Entities always have a transform component
					auto& tc = deserializedEntity.GetComponent<Components::Transform>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["Camera"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<Components::CameraComponent>();

					auto cameraProps = cameraComponent["SceneCamera"];
					//TODO serialize as string
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["Type"].as<int>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNearClip"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFarClip"].as<float>());

					cc.Camera.SetPerspectiveFov(cameraProps["PerspectiveFov"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNearClip"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFarClip"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRenderer"];
				if (spriteRendererComponent)
				{
					auto& sr = deserializedEntity.AddComponent<Components::SpriteRenderer>();

					sr.Color = spriteRendererComponent["Color"].as<glm::vec4>();
				}

				auto jsScriptComponent = entity["JSScript"];
				if (jsScriptComponent)
				{
					auto& jsScript = deserializedEntity.AddComponent<Components::JSScript>();

					jsScript.LoadScript(jsScriptComponent["Path"].as<std::string>());
				}

				auto rigidBodyComponent = entity["RigidBody2d"];
				if (rigidBodyComponent)
				{
					auto& rb = deserializedEntity.AddComponent<Components::RigidBody2d>();

					rb.FixedRotation = rigidBodyComponent["FixedRotation"].as<bool>();
					rb.Type = magic_enum::enum_cast<Components::RigidBody2d::BodyType>(rigidBodyComponent["Type"].as<std::string>()).value_or(Components::RigidBody2d::BodyType::Static);

					rb.Density = rigidBodyComponent["Density"].as<float>();
					rb.Friction = rigidBodyComponent["Friction"].as<float>();
					rb.Restitution = rigidBodyComponent["Restitution"].as<float>();
					rb.RestitutionThreshold = rigidBodyComponent["RestitutionThreshold"].as<float>();
				}

				auto boxColliderComponent = entity["BoxCollider2d"];
				if (boxColliderComponent)
				{
					auto& bc = deserializedEntity.AddComponent<Components::BoxCollider2d>();

					bc.Size = boxColliderComponent["Size"].as<glm::vec2>();
					bc.Offset = boxColliderComponent["Offset"].as<glm::vec2>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filePath)
	{
		AC_CORE_ASSERT(false, "Not implemented");
		return false;
	}
}