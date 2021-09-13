#pragma once

#include <Acorn.h>

#include "panels/ContentBrowser.h"
#include "panels/LogPanel.h"
#include "panels/SceneHierarchy.h"

#include <ImGuizmo.h>

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
			Translate = ImGuizmo::OPERATION::TRANSLATE,
			Rotate = ImGuizmo::OPERATION::ROTATE,
			Scale = ImGuizmo::OPERATION::SCALE,
		};

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

		//UI Panels
		void UI_Toolbar();

	private:
		struct WindowsOpen
		{
			bool Viewport = true;
			bool Entity = true;
			bool CameraControls = true;
			bool Stats = true;
			bool Logging = true;
		};

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
		};

		Ref<Texture2d>
			m_CheckerboardTexture;
		Ref<Texture2d> m_SpriteSheet;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_Square;
		Entity m_Camera;
		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		WindowsOpen m_WindowsOpen;
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize{0.0f};
		glm::vec2 m_ViewportBounds[2];

		SceneState m_SceneState = SceneState::Edit;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		std::shared_ptr<LogPanel> m_LogPanel;
		Ref<ContentBrowserPanel> m_ContentBrowserPanel;

		GizmoType m_GizmoType = GizmoType::Translate;

		std::string m_CurrentFilePath = "";
	};
}