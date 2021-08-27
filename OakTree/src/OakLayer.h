#pragma once

#include <Acorn.h>

namespace Acorn
{
	class OakLayer : public Layer
	{
	public:
		OakLayer();
		virtual ~OakLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender(Timestep t) override;
		virtual void OnEvent(Event& e) override;

	private:
		void SetupTerminal();

	private:
		struct WindowsOpen
		{
			bool Viewport = true;
			bool Settings = true;
			bool CameraControls = true;
			bool Stats = true;
			bool Logging = true;
		};
		OrthographicCameraController m_CameraController;

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		Ref<Texture2d> m_CheckerboardTexture;
		Ref<Texture2d> m_SpriteSheet;
		Ref<Framebuffer> m_Framebuffer;

		uint32_t m_MapWidth = 0, m_MapHeight = 0;
		std::unordered_map<char, Ref<ext2d::SubTexture>> m_TileMap;
		Ref<ext2d::SubTexture> m_BarrelTexture;

		//ImGui Debug Windows
		CustomCommandStruct m_CmdStruct;
		ImTerm::terminal<TerminalCommands> m_Terminal;

		WindowsOpen m_WindowsOpen;
		bool m_ViewportFocused = false, m_ViewportHovered = false;
	};
}