#include "OakLayer.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

static constexpr uint32_t s_MapWidth = 24;
static const char* s_MapTiles =
	"WWWWWWWWWWWWWWWWWWWWWWWW"
	"WWWWWWWWWWWWWWWWWWWWWWWW"
	"WWWWWWWWWGGGGGWWWWWWWWWW"
	"WWWWWWWWGGGGGGGWWWWWWWWW"
	"WWWWWWWGGGGGGGGWWWWWWWWW"
	"WWWWWWGGGGGGGGGGWWWWWWWW"
	"WWWWWGGGWWGGGGGGGWWWWWWW"
	"WWWWWGGGWWGGGGGGGGWWWWWW"
	"WWWWWGGGGGGGGGGGGGGWWWWW"
	"WWWWWWGGGGGGGGGGGGGWWWWW"
	"WWWWWWGGGGGGGGGGGGGWWWWW"
	"WWWWWWGGGGGGGGGGGGGWWWWW"
	"WWWWWWWWWWGGGGGGGGGWWWWW"
	"WWWWWWWWWWWWWWWGGGWWWWWW"
	"WWWWWWWWWWWWWWWWWWWWWWWW"
	"WWWWWWWWWWWWWWWWWWWWWWWW";

size_t GetCoords(size_t x, size_t y, uint32_t width)
{
	return x + y * (size_t)width;
}

namespace Acorn
{
	OakLayer::OakLayer()
		: Layer("Sandbox2D"), m_CameraController(16.0f / 9.0f, true)
	{
	}

	void OakLayer::OnAttach()
	{

		m_Framebuffer = Framebuffer::Create({Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight()});

		m_CheckerboardTexture = Texture2d::Create("res/textures/Checkerboard.png");
		m_SpriteSheet = Texture2d::Create("res/textures/RPGpack_sheet_2X.png");

		m_BarrelTexture = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {9, 0}, {128, 128});

		m_MapWidth = s_MapWidth;
		m_MapHeight = (uint32_t)strlen(s_MapTiles) / s_MapWidth;

		m_TileMap['G'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {1, 11}, {128, 128});
		m_TileMap['W'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {11, 11}, {128, 128});
		m_TileMap['L'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {10, 11}, {128, 128});
		m_TileMap['R'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {12, 11}, {128, 128});
		m_TileMap['B'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {11, 10}, {128, 128});
		m_TileMap['T'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {11, 12}, {128, 128});
		m_TileMap['H'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {10, 10}, {128, 128});
		m_TileMap['I'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {10, 12}, {128, 128});
		m_TileMap['J'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {12, 10}, {128, 128});
		m_TileMap['K'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {12, 12}, {128, 128});
		m_TileMap['M'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {13, 11}, {128, 128});
		m_TileMap['N'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {14, 11}, {128, 128});
		m_TileMap['O'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {13, 12}, {128, 128});
		m_TileMap['P'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {14, 12}, {128, 128});

		m_CameraController.SetZoomLevel(5.0f);

		m_ActiveScene = CreateRef<Scene>();

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

		//Panels

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_LogPanel = std::make_shared<LogPanel>();
		m_LogPanel->set_pattern("%^[%=8n][%T][%l]: %v%$");
		Log::AddSink(m_LogPanel);
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
		m_CameraController.OnEvent(e);
	}

}