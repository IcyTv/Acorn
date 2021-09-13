#include "OakLayer.h"
#include "utils/fonts/IconsFontAwesome4.h"

#include <Acorn/utils/fonts/IconsFontAwesome4.h>

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui.h>

namespace Acorn
{

	OakLayer::OakLayer()
		: Layer("Sandbox2D")
	{
	}

	void OakLayer::OnAttach()
	{

		FrameBufferSpecs fbSpecs;
		fbSpecs.Attachments = {FBTF::RGBA8, FBTF::R32I, FBTF::Depth};
		fbSpecs.Width = Application::Get().GetWindow().GetWidth();
		fbSpecs.Height = Application::Get().GetWindow().GetHeight();
		m_Framebuffer = Framebuffer::Create(fbSpecs);

		m_ActiveScene = CreateRef<Scene>();

		//Serialization
		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(sceneFilePath);
		}
		else
		{
			SceneSerializer serializer(m_ActiveScene);
			std::string defaultProject = "res/scenes/PinkCube.acorn";
			serializer.Deserialize(defaultProject);
			m_CurrentFilePath = defaultProject;
		}

		m_EditorCamera = EditorCamera(30.0f, 1.78f, 0.1f, 1000.0f);

		//Panels

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_LogPanel = std::make_shared<LogPanel>();
		m_LogPanel->set_pattern("%^[%=8n][%T][%l]: %v%$");
		Log::AddSink(m_LogPanel);

		m_ContentBrowserPanel = CreateRef<ContentBrowserPanel>();
	}

	void OakLayer::OnDetach()
	{
		AC_PROFILE_FUNCTION();
	}

	void OakLayer::OnUpdate(Timestep ts)
	{
		AC_PROFILE_FUNCTION();

		//Resize
		if (FrameBufferSpecs specs = m_Framebuffer->GetSpecs();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(specs.Width != m_ViewportSize.x || specs.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}

		if (m_ViewportFocused && m_SceneState == SceneState::Edit)
		{
			m_EditorCamera.OnUpdate(ts);
		}

		ext2d::Renderer::ResetStats();

		m_Framebuffer->Bind();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.2f, 1});
		RenderCommand::Clear();

		//Clear EntityId Attachment to -1
		m_Framebuffer->ClearColorAttachment(1, -1);

		//Update Scene

		if (m_SceneState == SceneState::Edit)
			m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
		else if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnUpdateRuntime(ts);

		if (m_SceneState == SceneState::Edit)
		{
			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			my = viewportSize.y - my;

			int mouseX = (int)mx;
			int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				int entityId = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
				m_HoveredEntity = Entity{entityId, m_ActiveScene.get()};
			}
		}

		m_Framebuffer->Unbind();
	}

	void OakLayer::OnImGuiRender(Timestep t)
	{
		AC_PROFILE_FUNCTION();

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSize;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}
				ImGui::Separator();

				if (ImGui::MenuItem("Quit"))
				{
					Application::Get().Close();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::BeginMenu("Gizmo Type"))
				{
					if (ImGui::MenuItem("None", "Q", m_GizmoType == GizmoType::None))
						m_GizmoType = GizmoType::None;

					if (ImGui::MenuItem("Translate", "W", m_GizmoType == GizmoType::Translate))
						m_GizmoType = GizmoType::Translate;

					if (ImGui::MenuItem("Rotate", "E", m_GizmoType == GizmoType::Rotate))
						m_GizmoType = GizmoType::Rotate;

					if (ImGui::MenuItem("Scale", "R", m_GizmoType == GizmoType::Scale))
						m_GizmoType = GizmoType::Scale;

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Viewport", NULL, &m_WindowsOpen.Viewport);
				ImGui::MenuItem("Entity", NULL, &m_WindowsOpen.Entity);
				ImGui::MenuItem("CameraControls", NULL, &m_WindowsOpen.CameraControls);
				ImGui::MenuItem("Stats", NULL, &m_WindowsOpen.Stats);
				ImGui::MenuItem("Logging", NULL, &m_WindowsOpen.Logging);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		//Panels
		m_SceneHierarchyPanel.OnImGuiRender();
		m_LogPanel->OnImGuiRender();
		m_ContentBrowserPanel->OnImGuiRender();

		if (m_WindowsOpen.Stats)
		{
			ImGui::Begin("Stats", &m_WindowsOpen.Stats);
			ImGui::Text("Renderer2d Stats");
			if (m_HoveredEntity)
				ImGui::Text("Hovered Entity %s", m_HoveredEntity.GetComponent<Components::Tag>().TagName.c_str());
			else
				ImGui::Text("Hovered Entity None");
			ImGui::Text("Quad Count %d", ext2d::Renderer::GetQuadCount());
			ImGui::Text("Draw Calls %d", ext2d::Renderer::GetDrawCalls());
			ImGui::Text("Vertices %d", ext2d::Renderer::GetVertexCount());
			ImGui::Text("Indices %d", ext2d::Renderer::GetIndexCount());
			ImGui::End();
		}

		if (m_WindowsOpen.Viewport)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			ImGui::Begin("Viewport", &m_WindowsOpen.Viewport);

			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();
			m_ViewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
			m_ViewportBounds[1] = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			// m_ViewportFocused |= m_ViewportHovered && ImGui::IsAnyMouseDown();
			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

			uint32_t viewportId = m_Framebuffer->GetColorAttachmentRendererId();
			ImVec2 availableSize = ImGui::GetContentRegionAvail();

			m_ViewportSize = {availableSize.x, availableSize.y};

			ImGui::Image((void*)(intptr_t)viewportId, availableSize, ImVec2{0, 1}, ImVec2{1, 0});

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");

				if (payload != nullptr)
				{
					auto path = (const wchar_t*)payload->Data;
					std::filesystem::path fsPath(path);

					OpenScene(path);
				}

				ImGui::EndDragDropTarget();
			}

			UI_Toolbar();
			// Gizmos
			Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			if (selectedEntity && m_GizmoType != GizmoType::None)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				// Camera
				// Runtime Camera
				// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				// const auto& camera = cameraEntity.GetComponent<Components::CameraComponent>().Camera;
				// const glm::mat4& cameraProjection = camera.GetProjection();
				// glm::mat4 cameraView = glm::inverse((glm::mat4)cameraEntity.GetComponent<Components::Transform>());

				// Editor Camera
				glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
				glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// Entity transform
				auto& tc = selectedEntity.GetComponent<Components::Transform>();
				glm::mat4 transform = tc;
				glm::vec3 originalRotation = tc.Rotation;

				//Snapping
				bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
				float snapValue = 0.5f; //Snap to 0.5 for everything else
				//Snap to 45 degrees for rotation
				if (m_GizmoType == GizmoType::Rotate)
				{
					snapValue = 45.0f;
				}

				float snapValues[3] = {snapValue, snapValue, snapValue};
				// Draw gizmos
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)(int)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
									 nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 translation, scale, skew;
					glm::vec4 perspective;
					glm::quat orientation;
					glm::decompose(transform, scale, orientation, translation, skew, perspective);

					tc.Translation = translation;

					tc.Rotation = glm::eulerAngles(orientation);
					tc.Scale = scale;
				}
			}

			// {
			// 	//Camera Icon
			// 	//TODO there is definitely a better way to do this
			// 	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
			// 	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
			// 	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

			// 	for (Entity entity : m_ActiveScene->GetEntitiesWithComponent<Components::CameraComponent>())
			// 	{
			// 		if (entity == m_SceneHierarchyPanel.GetSelectedEntity())
			// 			continue;
			// 		auto& transform = entity.GetComponent<Components::Transform>();
			// 		ImVec2 windowPos = ImGui::GetWindowPos();
			// 		ImVec2 windowSize = ImGui::GetWindowSize();

			// 		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
			// 		glm::vec3 pos = glm::project(translation, (glm::mat4)transform, m_EditorCamera.GetViewProjection(), glm::vec4{0.0f, 0.0f, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y});
			// 		pos.y = windowSize.y - pos.y;

			// 		ImVec2 charSize = ImGui::CalcTextSize(ICON_FA_CAMERA);
			// 		ImVec2 gizmoPos = ImVec2{pos.x - (charSize.x * 0.5f), pos.y - (charSize.y * 0.5f)};

			// 		if (gizmoPos.x > 0 && gizmoPos.x < m_ViewportSize.x - charSize.x && gizmoPos.y > 0 && gizmoPos.y < m_ViewportSize.y - charSize.y)
			// 		{
			// 			ImGui::SetCursorPos(gizmoPos);
			// 			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0, 0});
			// 			if (ImGui::Button(ICON_FA_CAMERA, charSize))
			// 			{
			// 				m_SceneHierarchyPanel.SetSelectedEntity(entity);
			// 			}
			// 			ImGui::PopStyleVar();
			// 		}
			// 	}
			// 	ImGui::PopStyleColor(3);
			// }

			ImGui::End();

			ImGui::PopStyleVar();
		}

		ImGui::End();
	}

	void OakLayer::OnEvent(Event& e)
	{
		AC_PROFILE_FUNCTION();

		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OakLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(OakLayer::OnMouseButtonPressed));
	}

	bool OakLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		//Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		if (e.GetModKeys() & (int)ModifierKey::Control && !ImGuizmo::IsUsing())
		{
			if (e.GetKeyCode() == KeyCode::N)
			{
				NewScene();
				return true;
			}
			else if (e.GetKeyCode() == KeyCode::S)
			{
				if (e.GetModKeys() & (int)ModifierKey::Shift)
				{
					SaveSceneAs();
					return true;
				}
				else
				{
					SaveScene();
					return true;
				}
			}
			else if (e.GetKeyCode() == KeyCode::O)
			{
				OpenScene();
				return true;
			}
		}
		else
		{
			switch (e.GetKeyCode())
			{
				//Gizmos
				case KeyCode::Q:
					m_GizmoType = GizmoType::None;
					return true;
				case KeyCode::W:
					m_GizmoType = GizmoType::Translate;
					return true;
				case KeyCode::E:
					m_GizmoType = GizmoType::Rotate;
					return true;
				case KeyCode::R:
					m_GizmoType = GizmoType::Scale;
					return true;

				//Camera
				case KeyCode::F:
				{
					auto entity = m_SceneHierarchyPanel.GetSelectedEntity();
					if (entity && entity.HasComponent<Components::Transform>())
					{
						auto& transform = entity.GetComponent<Components::Transform>();
						auto scale = transform.Scale;
						float zoom = std::max(scale.x, std::max(scale.y, scale.z)) + 1.0f;
						m_EditorCamera.SetFocalPointDistance(entity.GetComponent<Components::Transform>().Translation, zoom);
					}
					return true;
				}
				break;
				default:
					return false;
			}
		}
		return false;
	}

	bool OakLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (m_ViewportHovered && e.GetMouseButton() == AC_MOUSE_BUTTON_LEFT && !Input::IsKeyPressed(KeyCode::LeftAlt) && !ImGuizmo::IsOver())
		{
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			return true;
		}
		return false;
	}

	void OakLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilePath = "";
	}

	void OakLayer::SaveScene()
	{
		if (!m_CurrentFilePath.empty())
		{
			AC_CORE_INFO("Saving Scene to {}", m_CurrentFilePath);
			SceneSerializer(m_ActiveScene).Serialize(m_CurrentFilePath);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void OakLayer::SaveSceneAs()
	{
		std::string filename = PlatformUtils::SaveFile("Acorn Scene (*.acorn)\0*.acorn\0");
		if (!filename.empty())
		{
			AC_CORE_INFO("Saving Scene to {}", filename);
			m_CurrentFilePath = filename;
			SceneSerializer(m_ActiveScene).Serialize(filename);
		}
	}

	void OakLayer::OpenScene()
	{
		std::string filename = PlatformUtils::OpenFile("Acorn Scene (*.acorn)\0*.acorn\0");
		if (!filename.empty())
		{
			OpenScene(filename);
		}
	}

	void OakLayer::OpenScene(const std::filesystem::path& path)
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_CurrentFilePath = path.string();

		SceneSerializer(m_ActiveScene).Deserialize(path.string());
	}

	void OakLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
	}

	void OakLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
	}

	void OakLayer::UI_Toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto& colors = ImGui::GetStyle().Colors;
		auto& hoveredColor = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hoveredColor.x, hoveredColor.y, hoveredColor.z, 0.5f));
		auto& activeColor = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(activeColor.x, activeColor.y, activeColor.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		const char* sceneStateIcon = m_SceneState == SceneState::Edit ? ICON_FA_PLAY : ICON_FA_STOP;
		float size = ImGui::GetWindowHeight() - 4.0f;

		// static float initialLineHeight = ImGui::GetTextLineHeight();

		//TODO figure this out
		// ImGui::SetWindowFontScale(size / 16.0f - 0.2f);
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::Button(sceneStateIcon, ImVec2{size, size}))
		{
			if (m_SceneState == SceneState::Edit)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneStop();
		}
		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}
}