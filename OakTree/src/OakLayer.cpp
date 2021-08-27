#include "OakLayer.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <ImTerm/terminal.hpp>

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
"WWWWWWWWWWWWWWWWWWWWWWWW"
;

size_t GetCoords(size_t x, size_t y, uint32_t width)
{
	return x + y * (size_t)width;
}

namespace Acorn
{
	OakLayer::OakLayer()
		: Layer("Sandbox2D"), m_CameraController(16.0f / 9.0f, true),
		m_Terminal(ImTerm::terminal<TerminalCommands>(m_CmdStruct))
	{
	}

	void OakLayer::OnAttach()
	{
		m_Framebuffer = Framebuffer::Create({ Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight() });

		m_CheckerboardTexture = Texture2d::Create("res/textures/Checkerboard.png");
		m_SpriteSheet = Texture2d::Create("res/textures/RPGpack_sheet_2X.png");

		m_BarrelTexture = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 9, 0 }, { 128, 128 });

		m_MapWidth = s_MapWidth;
		m_MapHeight = (uint32_t)strlen(s_MapTiles) / s_MapWidth;

		m_TileMap['G'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {  1, 11 }, { 128, 128 });
		m_TileMap['W'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 });
		m_TileMap['L'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 11 }, { 128, 128 });
		m_TileMap['R'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 11 }, { 128, 128 });
		m_TileMap['B'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 10 }, { 128, 128 });
		m_TileMap['T'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 12 }, { 128, 128 });
		m_TileMap['H'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 10 }, { 128, 128 });
		m_TileMap['I'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 12 }, { 128, 128 });
		m_TileMap['J'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 10 }, { 128, 128 });
		m_TileMap['K'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 12 }, { 128, 128 });
		m_TileMap['M'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 13, 11 }, { 128, 128 });
		m_TileMap['N'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 14, 11 }, { 128, 128 });
		m_TileMap['O'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 13, 12 }, { 128, 128 });
		m_TileMap['P'] = ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 14, 12 }, { 128, 128 });

		m_CameraController.SetZoomLevel(5.0f);

		SetupTerminal();
	}

	void OakLayer::OnDetach()
	{
		AC_PROFILE_FUNCTION();

		Log::RemoveTerminal(m_Terminal.get_terminal_helper());

	}

	void OakLayer::OnUpdate(Timestep ts)
	{
		AC_PROFILE_FUNCTION();

		m_Framebuffer->Bind();
		if (m_ViewportFocused || m_CameraController.ShouldUpdate())
		{
			m_CameraController.OnUpdate(ts);
		}

		ext2d::Renderer::ResetStats();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.2f, 1 });
		RenderCommand::Clear();
	
	#if 0
		static float rotation = 0.0f;
		rotation += ts * 40.0f;

		ext2d::Renderer::BeginScene(m_CameraController.GetCamera());

		ext2d::Renderer::FillQuad({ -1.0f, 0.0f }, { 0.8f, 1.2f }, { 1.0f, 0.0f, 0.0f, 1.0f });
		ext2d::Renderer::FillQuad({ -0.5f, -1.0f }, { 1.0f, 1.0f }, m_SquareColor);
		ext2d::Renderer::FillRotatedQuad({ -1.5f, -1.5f }, { 1.0f, 1.0f }, glm::radians(45.0f), {0.0f, 1.0f, 0.0f, 1.0f});
		ext2d::Renderer::FillQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10.0f);
		ext2d::Renderer::FillRotatedQuad({ 0.2f, 0.0f, 0.1f }, { 1.0f, 1.0f }, glm::radians(rotation), m_CheckerboardTexture, 20.0f);
		ext2d::Renderer::EndScene();
	#endif
		ext2d::Renderer::BeginScene(m_CameraController.GetCamera());

		for (size_t y = 0; y < m_MapHeight; y++)
		{
			for (size_t x = 0; x < m_MapWidth; x++)
			{
				Ref<ext2d::SubTexture> texture = nullptr;

				char tile = s_MapTiles[x + y * m_MapWidth];
				if (m_TileMap.find(tile) != m_TileMap.end())
				{
					texture = m_TileMap[tile];
					//ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);
				}
				else
				{
					ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
					return;
				}

				if (x > 0 && x < m_MapWidth - 1 && y > 0 && y < m_MapHeight - 1 && tile != 'G')
				{
					bool neighbors[8] =
					{
						s_MapTiles[GetCoords(x - 1, y    , m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x + 1, y    , m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x    , y - 1, m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x    , y + 1, m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x - 1, y - 1, m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x + 1, y - 1, m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x - 1, y + 1, m_MapWidth)] != tile,
						s_MapTiles[GetCoords(x + 1, y + 1, m_MapWidth)] != tile,
					};


					if (neighbors[4])
						texture = m_TileMap['P'];
					if (neighbors[5])
						texture = m_TileMap['O'];
					if (neighbors[6])
						texture = m_TileMap['N'];
					if (neighbors[7])
						texture = m_TileMap['M'];

					if (neighbors[0])
						texture = m_TileMap['L'];
					if (neighbors[1])
						texture = m_TileMap['R'];
					if (neighbors[2])
						texture = m_TileMap['B'];
					if (neighbors[3])
						texture = m_TileMap['T'];

					if (neighbors[0] && neighbors[2])
						texture = m_TileMap['H'];
					if (neighbors[0] && neighbors[3])
						texture = m_TileMap['I'];
					if (neighbors[1] && neighbors[2])
						texture = m_TileMap['J'];
					if (neighbors[1] && neighbors[3])
						texture = m_TileMap['K'];
				}

				ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);

			}
		}

		ext2d::Renderer::FillQuad({ 0, 0, 0.05f }, { 1.0f, 1.0f }, m_BarrelTexture);

		ext2d::Renderer::EndScene();

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
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

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
				ImGui::MenuItem("Settings", NULL, &m_WindowsOpen.Settings);
				ImGui::MenuItem("CameraControls", NULL, &m_WindowsOpen.CameraControls);
				ImGui::MenuItem("Stats", NULL, &m_WindowsOpen.Stats);
				ImGui::MenuItem("Logging", NULL, &m_WindowsOpen.Logging);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_WindowsOpen.Settings)
		{
			ImGui::Begin("Settings", &m_WindowsOpen.Settings);
			ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
			ImGui::End();
		}

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

		m_CameraController.ImGuiControls(&m_WindowsOpen.CameraControls);


		if (m_WindowsOpen.Logging)
		{
			if (ImGui::Begin("Logging", &m_WindowsOpen.Logging, ImGuiWindowFlags_NoScrollbar))
			{
				m_WindowsOpen.Logging = m_Terminal.show();
			}
			ImGui::End();
		}

		if (m_WindowsOpen.Viewport)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			if (ImGui::Begin("Viewport", &m_WindowsOpen.Viewport))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

				uint32_t viewportId = m_Framebuffer->GetColorAttachmentRendererId();
				ImVec2 availableSize = ImGui::GetContentRegionAvail();

				if ((availableSize.x != m_Framebuffer->GetSpecs().Width || availableSize.y != m_Framebuffer->GetSpecs().Height) && availableSize.x > 0 && availableSize.y > 0)
				{
					//TODO debounce?
					m_Framebuffer->Resize((uint32_t)availableSize.x, (uint32_t)availableSize.y);
					m_CameraController.ResizeBounds(availableSize.x, availableSize.y);
				}

				ImGui::Image((void*)(intptr_t)viewportId, availableSize, ImVec2{ 0,1 }, ImVec2{ 1,0 });
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

	void OakLayer::SetupTerminal()
	{
		Log::SetTerminal(m_Terminal.get_terminal_helper());

		m_Terminal.theme().log_level_colors[ImTerm::message::severity::info] = { 0.0f, 1.0f, 0.0f, 1.0f };
		m_Terminal.theme().log_level_colors[ImTerm::message::severity::warn] = { 0.98f, 0.91f, 0.3f, 1.0f };
		m_Terminal.theme().log_level_colors[ImTerm::message::severity::err] = { 0.7f, 0.0f, 0.0f, 1.0f };
		m_Terminal.theme().log_level_colors[ImTerm::message::severity::critical] = { 1.0f, 1.0f, 0.0f, 1.0f };
	}

}