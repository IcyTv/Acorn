#pragma once

#include "panels/LogPanel.h"
#include "panels/SceneHierarchy.h"
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
		enum class GizmoType : int
		{
			None = -1,
			Translate = 0,
			Rotate = 1,
			Scale = 2,
		};

	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void OpenScene();

	private:
		struct WindowsOpen
		{
			bool Viewport = true;
			bool Entity = true;
			bool CameraControls = true;
			bool Stats = true;
			bool Logging = true;
		};
		OrthographicCameraController m_CameraController;

		Ref<Texture2d> m_CheckerboardTexture;
		Ref<Texture2d> m_SpriteSheet;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_Square;
		Entity m_Camera;

		WindowsOpen m_WindowsOpen;
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize{0.0f};

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		std::shared_ptr<LogPanel> m_LogPanel;

		GizmoType m_GizmoType = GizmoType::Translate;

		std::string m_CurrentFilePath = "";
	};
}