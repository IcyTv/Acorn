#include "SceneHierarchy.h"
#include "ecs/components/Components.h"
#include "ecs/components/TSCompiler.h"
#ifndef NO_SCRIPTING
#include "ecs/components/V8Script.h"
#endif // !NO_SCRIPTING

#include <filesystem>
#include <fmt/core.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <magic_enum.hpp>
#include <string.h>
#include <string_view>

namespace Acorn
{
	constexpr size_t TAG_SIZE = 256;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context) : m_Context(context) {}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context		   = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		AC_PROFILE_FUNCTION();
		ImGui::Begin("Scene Hierarchy");
		// ImGui::BeginChild("entity_drop_target", ImVec2(0, -1), false);

		// Getting all entities that do not have a parent
		auto view = m_Context->m_Registry.view<Components::ID>(entt::exclude<Components::ParentRelationship>);

		for (auto entityID : view)
		{
			Entity entity{ entityID, m_Context.get() };

			DrawEntityNode(entity);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsItemHovered())
		{
			m_SelectionContext = {};
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}

		// ImGui::EndChild();

		ImVec2 availableSize = ImGui::GetContentRegionAvail();
		ImGui::Dummy(availableSize);

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");

			if (payload != nullptr)
			{
				Entity child = *(Entity*) payload->Data;
				AC_CORE_INFO("Moving {} to root", child.GetName());
				if (child.HasComponent<Components::ParentRelationship>())
				{
					auto& parentComp = child.GetComponent<Components::ParentRelationship>();
					Entity parent	 = parentComp.Parent;
					AC_CORE_ASSERT(parent.HasComponent<Components::ChildRelationship>(), "Parent does not have any children.");
					auto& childComp = parent.GetComponent<Components::ChildRelationship>();
					childComp.RemoveEntity(child);

					if (childComp.Empty())
					{
						parent.RemoveComponent<Components::ParentRelationship>();
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		ImGui::BeginChild("properties_drop_target", ImVec2(0, -1), false);

		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		else
			ImGui::Text("No Selection");

		ImGui::EndChild();
		if (m_SelectionContext && ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("JS_SCRIPT_FILE");

			if (payload != nullptr)
			{
				auto path = (const wchar_t*) payload->Data;
				std::filesystem::path fsPath(path);

				m_SelectionContext.AddComponent<Components::JSScript>(m_SelectionContext, fsPath.string());
				// jsScript.LoadScript(fsPath.string());
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		std::string& tag = entity.GetComponent<Components::Tag>().TagName;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth |
			((!entity.HasComponent<Components::ChildRelationship>() || entity.GetComponent<Components::ChildRelationship>().Empty()) ? ImGuiTreeNodeFlags_Leaf : 0) |
			((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0);

		bool opened = ImGui::TreeNodeEx((void*) (intptr_t) (uint32_t) entity, flags, "%s", tag.c_str());

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("Entity", &entity, sizeof(Entity));
			ImGui::Text("%s", tag.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");

			if (payload != nullptr)
			{
				AC_CORE_INFO("DragDropTarget called");
				Entity* target = (Entity*) payload->Data;
				if (target->GetUUID() != entity.GetUUID())
				{
					if (!entity.HasComponent<Components::ChildRelationship>())
					{
						entity.AddComponent<Components::ChildRelationship>();
					}
					auto& rel = entity.GetComponent<Components::ChildRelationship>();
					rel.AddEntity(entity, *target, m_Context);
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				entityDeleted = true;
			}

			if (ImGui::MenuItem("Duplicate"))
			{
				m_Context->DuplicateEntity(entity);
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			if (entity.HasComponent<Components::ChildRelationship>())
			{
				auto children = entity.GetComponent<Components::ChildRelationship>().Entities;

				for (auto child : children)
				{
					DrawEntityNode(child);
				}
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	static bool DrawVec3Control(std::string_view label, glm::vec3& vec, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		AC_PROFILE_FUNCTION();
		ImGuiIO& io	  = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		bool changed = false;

		// ImGui::PushID(label.c_str());
		// ImGui::Columns(2);
		// ImGui::SetColumnWidth(0, columnWidth);
		// ImGui::Text("%s", label.c_str());
		// ImGui::NextColumn();
		// Acorn::GUI::BeginTable(label);
		Acorn::GUI::Item(label,
			[&]()
			{
				ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

				float lineHeight  = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
				ImGui::PushFont(boldFont);
				if (ImGui::Button("X", buttonSize))
				{
					changed = true;
					vec.x	= resetValue;
				}
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				changed |= ImGui::DragFloat("##X", &vec.x, 0.1f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Y", buttonSize))
				{
					changed = true;
					vec.y	= resetValue;
				}
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				changed |= ImGui::DragFloat("##Y", &vec.y, 0.1f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();
				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
				ImGui::PushFont(boldFont);
				if (ImGui::Button("Z", buttonSize))
				{
					vec.z	= resetValue;
					changed = true;
				}
				ImGui::PopFont();
				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				changed |= ImGui::DragFloat("##Z", &vec.z, 0.1f, 0.0f, 0.0f, "%.2f");
				ImGui::PopItemWidth();

				ImGui::PopStyleVar();
			});

		// ImGui::Columns(1);
		// ImGui::PopID();
		// Acorn::GUI::EndTable();

		return changed;
	}

	template <typename T, typename UIFunc>
	static void DrawComponent(std::string_view label, Entity entity, UIFunc func)
	{
		AC_PROFILE_FUNCTION();
		constexpr ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();

			float contentRegionX = ImGui::GetContentRegionAvailWidth();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			ImGui::Separator();

			bool open = ImGui::TreeNodeEx((void*) typeid(T).hash_code(), flags, "%s", label.data());

			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			ImGui::PopStyleVar();

			ImGui::SameLine(contentRegionX - lineHeight);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
				{
					removeComponent = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			if (open)
			{
				Acorn::GUI::BeginTable(label);
				func(component);
				Acorn::GUI::EndTable();

				ImGui::TreePop();
			}

			if (removeComponent)
			{
				entity.RemoveComponent<T>();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		AC_PROFILE_FUNCTION();
		if (entity.HasComponent<Components::Tag>())
		{
			auto& tag = entity.GetComponent<Components::Tag>().TagName;
			char buffer[TAG_SIZE];
			memset(buffer, 0, sizeof(buffer));
			size_t len = strlen(tag.c_str());
			memcpy(buffer, tag.c_str(), len);
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (!entity.HasComponent<Components::SpriteRenderer>())
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					entity.AddComponent<Components::SpriteRenderer>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::SpriteRenderer>())
			{
				if (ImGui::MenuItem("Circle Renderer"))
				{
					entity.AddComponent<Components::CircleRenderer>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::CameraComponent>())
			{
				if (ImGui::MenuItem("Camera"))
				{
					entity.AddComponent<Components::CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::JSScript>())
			{
				if (ImGui::MenuItem("JS Script"))
				{
					entity.AddComponent<Components::JSScript>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::RigidBody2d>())
			{
				if (ImGui::MenuItem("Rigid Body 2d"))
				{
					entity.AddComponent<Components::RigidBody2d>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::BoxCollider2d>())
			{
				if (ImGui::MenuItem("Box Collider 2d"))
				{
					entity.AddComponent<Components::BoxCollider2d>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!entity.HasComponent<Components::CircleCollider2d>())
			{
				if (ImGui::MenuItem("Circle Collider 2d"))
				{
					entity.AddComponent<Components::CircleCollider2d>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<Components::Transform>("Transform", entity,
			[](auto& transformComponent)
			{
				DrawVec3Control("Position"sv, transformComponent.Translation);
				glm::vec3 rotation = glm::degrees(transformComponent.Rotation);
				if (DrawVec3Control("Rotation"sv, rotation))
				{
					transformComponent.Rotation = glm::radians(rotation);
				}
				DrawVec3Control("Scale"sv, transformComponent.Scale, 1.0f);
			});

		DrawComponent<Components::CameraComponent>("Camera", entity,
			[](auto& cameraComponent)
			{
				auto& camera = cameraComponent.Camera;

				Acorn::GUI::Checkbox("##Primary Camera"sv, cameraComponent.Primary);

				auto projection = camera.GetProjectionType();
				if (Acorn::GUI::Combo("##Projection Type"sv, projection,
						{ { SceneCamera::ProjectionType::Orthographic, "Orthographic"sv }, { SceneCamera::ProjectionType::Perspective, "Perspective"sv } }))
				{
					camera.SetProjectionType(projection);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					Acorn::GUI::Drag("##Size"sv, camera.OrthographicSize(), 0.1f);
					Acorn::GUI::Drag("##Near Plane"sv, camera.OrthographicNearClip(), 0.1f);
					Acorn::GUI::Drag("##Far Plane"sv, camera.OrthographicFarClip(), 0.1f);

					Acorn::GUI::Checkbox("##Fixed Aspect Ratio", cameraComponent.FixedAspectRatio);
				}
				else
				{
					float fov = glm::degrees(camera.GetPerspectiveFov());
					if (Acorn::GUI::Drag("##Field Of View"sv, fov, 1.0f))
					{
						camera.SetPerspectiveFov(glm::radians(fov));
					}

					Acorn::GUI::Drag("##Near Plane"sv, camera.PerspectiveNearClip(), 0.1f);
					Acorn::GUI::Drag("##Far Plane"sv, camera.PerspectiveFarClip(), 0.1f);
				}
			});

		DrawComponent<Components::SpriteRenderer>("Sprite Renderer", entity,
			[&](Components::SpriteRenderer& spriteRenderer)
			{
				Acorn::GUI::ColorEdit("##Color"sv, spriteRenderer.Color);
				// Texture
				if (spriteRenderer.Texture)
				{
					Acorn::GUI::ImageButton("##Texture", spriteRenderer.Texture);
				}
				else
				{
					Acorn::GUI::TableButton("##Texture");
				}

				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					ImGui::OpenPopup("TexturePopup");

				if (ImGui::BeginPopup("TexturePopup"))
				{
					if (spriteRenderer.Texture)
					{
						if (ImGui::Button("Remove Texture"))
						{
							spriteRenderer.Texture = nullptr;
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::End();
				}

				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET");

					if (payload != nullptr)
					{
						auto path = (const wchar_t*) payload->Data;
						std::filesystem::path fsPath(path);
						spriteRenderer.Texture = Texture2d::Create(fsPath.string());
					}

					ImGui::EndDragDropTarget();
				}

				// ImGui::DragFloat("Tiling Factor", &spriteRenderer.TilingFactor, 0.1f, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
				if (spriteRenderer.Texture)
					Acorn::GUI::Drag("##Tiling Factor", spriteRenderer.TilingFactor, 0.1f);
			});

		DrawComponent<Components::CircleRenderer>("Circle Renderer", entity,
			[&](Components::CircleRenderer& circleRenderer)
			{
				Acorn::GUI::ColorEdit("##Color", circleRenderer.Color);
				// ImGui::DragFloat("Radius", &circleRenderer.Radius, 0.1f, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
				// ImGui::DragFloat("Thickness", &circleRenderer.Thickness, 0.01f, 0.0f, 1.0f, "%.3f");
				// ImGui::DragFloat("Fade", &circleRenderer.Fade, 0.00025f, 0.0f, 1.0f, "%.6f");
				Acorn::GUI::Drag("##Fade", circleRenderer.Fade, 0.01f);
				Acorn::GUI::Drag("##Thickness", circleRenderer.Thickness, 0.1f);
			});

#ifndef NO_SCRIPTING
		DrawComponent<Components::JSScript>("JS Script", entity,
			[&](Components::JSScript& jsScript)
			{
				// TODO style
				char scriptName[256] = { 0 };
				if (jsScript.Script && jsScript.Script->GetFilePath().size() > 0)
				{
					strcpy_s(scriptName, jsScript.Script->GetFilePath().c_str());
				}
				ImGui::InputText("Script Path", scriptName, sizeof(scriptName));

				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("JS_SCRIPT_FILE");

					if (payload != nullptr)
					{
						auto path = (const wchar_t*) payload->Data;
						std::filesystem::path fsPath(path);
						jsScript.LoadScript(entity, fsPath.string());
					}

					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();

				if (ImGui::Button("Load Script"))
				{
					jsScript.LoadScript(entity, scriptName);
				}

#if 0
				if (jsScript.Script)
				{
					TSScriptData scriptData = jsScript.Script->GetScriptData();

					ImGui::Text("%s", scriptData.ClassName.c_str());
					ImGui::PushID(scriptData.ClassName.c_str());

					for (auto& field : scriptData.Fields)
					{
						ImGui::Text("%s", field.first.c_str());
						ImGui::SameLine();

						switch (ToV8Type(field.second.Type))
						{
							case V8Types::Boolean:
							{
								ImGui::Checkbox(fmt::format("##{}", field.first.c_str()).c_str(), &jsScript.Script->GetValue<bool>(field.first));
							}
							break;
							case V8Types::Number:
							{
								ImGui::DragFloat(fmt::format("##{}", field.first.c_str()).c_str(), &jsScript.Script->GetValue<float>(field.first), 0.1f, -100.0f, 100.0f, "%.2f", ImGuiSliderFlags_None);
							}
							break;
							case V8Types::String:
							{
								// TODO do once
								std::string value = jsScript.Script->GetValue<std::string>(field.first);
								char buf[256] = {0};
								strcpy_s(buf, value.c_str());
								if (ImGui::InputText(fmt::format("##{}", field.first.c_str()).c_str(), buf, 256))
								{
									jsScript.Script->SetValue<std::string>(field.first, std::string(buf));
								}
							}
							break;
							case V8Types::Unknown:
							{
								ImGui::Text("Unknown Type, try specifying as a type");
							}
						}
					}
					ImGui::PopID();
				}
#endif
			});
#endif
		DrawComponent<Components::RigidBody2d>("RigidBody", entity,
			[&](Components::RigidBody2d& rigidBody)
			{
				using namespace Components;
				Acorn::GUI::Combo("##Body Type", rigidBody.Type,
					{ { RigidBody2d::BodyType::Static, "Static" }, { RigidBody2d::BodyType::Dynamic, "Dynamic" }, { RigidBody2d::BodyType::Kinematic, "Kinematic" } });

				ImGui::Separator();

				Acorn::GUI::Checkbox("##Fixed Rotation"sv, rigidBody.FixedRotation);

				ImGui::Separator();

				Acorn::GUI::Drag("##Density"sv, rigidBody.Density, 0.01f, 0.0f, 1.0f, "%.3f"sv);
				Acorn::GUI::Drag("##Friction"sv, rigidBody.Friction, 0.01f, 0.0f, 1.0f, "%.3f"sv);
				Acorn::GUI::Drag("##Restitution"sv, rigidBody.Restitution, 0.01f, 0.0f, 1.0f, "%.3f"sv);
				Acorn::GUI::Drag("##Restitution Threshold"sv, rigidBody.RestitutionThreshold, 0.01f, 0.0f, 1.0f, "%.3f"sv);
			});

		DrawComponent<Components::BoxCollider2d>("BoxCollider", entity,
			[&](Components::BoxCollider2d& collider)
			{
				Acorn::GUI::Drag("##Size"sv, collider.GetSize());
				Acorn::GUI::Drag("##Offset"sv, collider.GetOffset());
			});

		DrawComponent<Components::CircleCollider2d>("CircleCollider", entity,
			[&](Components::CircleCollider2d& collider)
			{
				Acorn::GUI::Drag("##Radius"sv, collider.GetRadius(), 0.01f, 0.0f, 10.0f, "%.2f"sv);
				Acorn::GUI::Drag("##Offset"sv, collider.GetOffset(), 0.01f, -10.0f, 10.0f, "%.2f"sv);
			});
	}
} // namespace Acorn