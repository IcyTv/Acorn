#include "OakLayer.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "serialize/Serializer.h"
#include "utils/PlatformUtils.h"

namespace Acorn
{
	OakLayer::OakLayer()
		: Layer("Sandbox2D"), m_CameraController(16.0f / 9.0f, true)
	{
	}

	void OakLayer::OnAttach()
	{

		m_Framebuffer = Framebuffer::Create({Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight()});

		m_ActiveScene = CreateRef<Scene>();

#if 0
		auto square = m_ActiveScene->CreateEntity("Square");

		square.AddComponent<Components::SpriteRenderer>(glm::vec4{1.0f, 1.0f, 0.0f, 1.0f});

		m_Square = square;

		m_Camera = m_ActiveScene->CreateEntity("Camera");
		m_Camera.AddComponent<Components::CameraComponent>();

		class CameraController : public ScriptableEntity
		{
		public:
			void OnCreate() override
			{
				AC_INFO("OnCreate!");
				srand((uint32_t)time(NULL));
				auto& transform = GetComponent<Components::Transform>().Translation;
				transform.x = (float)(rand() % 10 - 5.0f);
			}

			void OnUpdate(Timestep ts) override
			{
				auto& transform = GetComponent<Components::Transform>().Translation;
				auto& rotation = GetComponent<Components::Transform>().Rotation;

				if (Input::IsKeyPressed(AC_KEY_A))
				{
					transform.x -= 10.0f * ts;
				}
				if (Input::IsKeyPressed(AC_KEY_D))
				{
					transform.x += 10.0f * ts;
				}
				if (Input::IsKeyPressed(AC_KEY_W))
				{
					transform.y += 10.0f * ts;
				}
				if (Input::IsKeyPressed(AC_KEY_S))
				{
					transform.y -= 10.0f * ts;
				}

				if (Input::IsKeyPressed(AC_KEY_Q))
				{
					rotation.z += glm::radians(45.0f * ts);
				}
				if (Input::IsKeyPressed(AC_KEY_E))
				{
					rotation.z -= glm::radians(45.0f * ts);
				}
			}
		};

		m_Camera.AddComponent<Components::NativeScript>().Bind<CameraController>();

#endif
		//Panels

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_LogPanel = std::make_shared<LogPanel>();
		m_LogPanel->set_pattern("%^[%=8n][%T][%l]: %v%$");
		Log::AddSink(m_LogPanel);

		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize("res/scenes/Example.acorn");
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
			m_CameraController.ResizeBounds(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_Framebuffer->Bind();
		if (m_ViewportFocused || m_CameraController.ShouldUpdate())
		{
			m_CameraController.OnUpdate(ts);
		}

		ext2d::Renderer::ResetStats();

		RenderCommand::SetClearColor({0.1f, 0.1f, 0.2f, 1});
		RenderCommand::Clear();

		//Update Scene

		m_ActiveScene->OnUpdate(ts);

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

		if (m_WindowsOpen.Stats)
		{
			ImGui::Begin("Stats", &m_WindowsOpen.Stats);
			auto stats = ext2d::Renderer::GetStats();
			ImGui::Text("Renderer2d Stats");
			ImGui::Text("Quad Count %d", stats.QuadCount);
			ImGui::Text("Draw Calls %d", stats.DrawCalls);
			ImGui::Text("Vertices %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices %d", stats.GetTotalIndexCount());
			ImGui::End();
		}

		// m_CameraController.ImGuiControls(&m_WindowsOpen.CameraControls);

		if (m_WindowsOpen.Viewport)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			if (ImGui::Begin("Viewport", &m_WindowsOpen.Viewport))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

				uint32_t viewportId = m_Framebuffer->GetColorAttachmentRendererId();
				ImVec2 availableSize = ImGui::GetContentRegionAvail();

				m_ViewportSize = {availableSize.x, availableSize.y};

				ImGui::Image((void*)(intptr_t)viewportId, availableSize, ImVec2{0, 1}, ImVec2{1, 0});
			}
			ImGui::End();

			ImGui::PopStyleVar();
		}

		ImGui::End();
	}

	void OakLayer::OnEvent(Event& e)
	{
		AC_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OakLayer::OnKeyPressed));
	}

	bool OakLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		//Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		if (e.GetModKeys() & (int)ModifierKey::Control)
		{
			if (e.GetKeyCode() == KeyCode::N)
			{
				NewScene();
			}
			else if (e.GetKeyCode() == KeyCode::S)
			{
				if (e.GetModKeys() & (int)ModifierKey::Shift)
				{
					SaveSceneAs();
				}
			}
			else if (e.GetKeyCode() == KeyCode::O)
			{
				OpenScene();
			}
		}
		return true;
	}

	void OakLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void OakLayer::SaveSceneAs()
	{
		std::string filename = PlatformUtils::SaveFile("Acorn Scene (*.acorn)\0*.acorn\0");
		if (!filename.empty())
			SceneSerializer(m_ActiveScene).Serialize(filename);
	}

	void OakLayer::OpenScene()
	{
		std::string filename = PlatformUtils::OpenFile("Acorn Scene (*.acorn)\0*.acorn\0");
		if (!filename.empty())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer(m_ActiveScene).Deserialize(filename);
		}
	}
}