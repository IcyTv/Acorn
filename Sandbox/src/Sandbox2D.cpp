#include "Sandbox2D.h"

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
"WWWWWWWWWWWWWWWWWWWWWWWW"
;

size_t GetCoords(size_t x, size_t y, uint32_t width)
{
	return x + y * (size_t)width;
}

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(16.0f / 9.0f, true)
{

}

void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = Acorn::Texture2d::Create("res/textures/Checkerboard.png");
	m_SpriteSheet = Acorn::Texture2d::Create("res/textures/RPGpack_sheet_2X.png");

	m_BarrelTexture = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 9, 0 }, { 128, 128 });

	m_MapWidth = s_MapWidth;
	m_MapHeight = (uint32_t)strlen(s_MapTiles) / s_MapWidth;

	m_TileMap['G'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, {  1, 11 }, { 128, 128 });
	m_TileMap['W'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 });
	m_TileMap['L'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 11 }, { 128, 128 });
	m_TileMap['R'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 11 }, { 128, 128 });
	m_TileMap['B'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 10 }, { 128, 128 });
	m_TileMap['T'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 11, 12 }, { 128, 128 });
	m_TileMap['H'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 10 }, { 128, 128 });
	m_TileMap['I'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 10, 12 }, { 128, 128 });
	m_TileMap['J'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 10 }, { 128, 128 });
	m_TileMap['K'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 12, 12 }, { 128, 128 });
	m_TileMap['M'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 13, 11 }, { 128, 128 });
	m_TileMap['N'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 14, 11 }, { 128, 128 });
	m_TileMap['O'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 13, 12 }, { 128, 128 });
	m_TileMap['P'] = Acorn::ext2d::SubTexture::CreateFromCoords(m_SpriteSheet, { 14, 12 }, { 128, 128 });

	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.1f, m_Particle.SizeVariation = 0.1f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 5.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	m_CameraController.SetZoomLevel(5.0f);

}

void Sandbox2D::OnDetach()
{
	AC_PROFILE_FUNCTION();

}

void Sandbox2D::OnUpdate(Acorn::Timestep ts)
{
	AC_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	Acorn::ext2d::Renderer::ResetStats();

	Acorn::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.2f, 1 });
	Acorn::RenderCommand::Clear();
	
#if 0
	static float rotation = 0.0f;
	rotation += ts * 40.0f;

	Acorn::ext2d::Renderer::BeginScene(m_CameraController.GetCamera());

	Acorn::ext2d::Renderer::FillQuad({ -1.0f, 0.0f }, { 0.8f, 1.2f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	Acorn::ext2d::Renderer::FillQuad({ -0.5f, -1.0f }, { 1.0f, 1.0f }, m_SquareColor);
	Acorn::ext2d::Renderer::FillRotatedQuad({ -1.5f, -1.5f }, { 1.0f, 1.0f }, glm::radians(45.0f), {0.0f, 1.0f, 0.0f, 1.0f});
	Acorn::ext2d::Renderer::FillQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10.0f);
	Acorn::ext2d::Renderer::FillRotatedQuad({ 0.2f, 0.0f, 0.1f }, { 1.0f, 1.0f }, glm::radians(rotation), m_CheckerboardTexture, 20.0f);
	Acorn::ext2d::Renderer::EndScene();
#endif

	if (Acorn::Input::IsMouseButtonPressed(AC_MOUSE_BUTTON_LEFT))
	{
		auto [x, y] = Acorn::Input::GetMousePosition();
		auto width = Acorn::Application::Get().GetWindow().GetWidth();
		auto height = Acorn::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 500.0f * ts; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

	m_Particle.Position = { 0.0f, 0.0f };
	for (int i = 0; i < 50.0f * ts; i++)
		m_ParticleSystem.Emit(m_Particle);


	Acorn::ext2d::Renderer::BeginScene(m_CameraController.GetCamera());

	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());


	for (size_t y = 0; y < m_MapHeight; y++)
	{
		for (size_t x = 0; x < m_MapWidth; x++)
		{
			Acorn::Ref<Acorn::ext2d::SubTexture> texture = nullptr;

			char tile = s_MapTiles[x + y * m_MapWidth];
			if (m_TileMap.find(tile) != m_TileMap.end())
			{
				texture = m_TileMap[tile];
				//Acorn::ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);
			}
			else
			{
				Acorn::ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
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

			Acorn::ext2d::Renderer::FillQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f }, { 1.0f, 1.0f }, texture);

		}
	}

	Acorn::ext2d::Renderer::FillQuad({ 0, 0, 0.05f }, { 1.0f, 1.0f }, m_BarrelTexture);

	Acorn::ext2d::Renderer::EndScene();
}

void Sandbox2D::OnImGuiRender(Acorn::Timestep t)
{
	AC_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();

	ImGui::Begin("Stats");
	auto stats = Acorn::ext2d::Renderer::GetStats();
	ImGui::Text("Renderer2d Stats");
	ImGui::Text("Quad Count %d", stats.QuadCount);
	ImGui::Text("Draw Calls %d", stats.DrawCalls);
	ImGui::Text("Vertices %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices %d", stats.GetTotalIndexCount());

	ImGui::End();

	m_CameraController.ImGuiControls(&m_IsCameraControlsOpen);
}

void Sandbox2D::OnEvent(Acorn::Event& e)
{
	AC_PROFILE_FUNCTION();
	m_CameraController.OnEvent(e);
}
