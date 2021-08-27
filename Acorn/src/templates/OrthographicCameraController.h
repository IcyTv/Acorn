#pragma once

#include "renderer/Camera.h"
#include "core/Timestep.h"
#include "events/Event.h"
#include "events/ApplicationEvent.h"
#include "events/MouseEvent.h"

#include <glm/glm.hpp>

namespace Acorn
{
	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false); //aspectratio * 2 units

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void ImGuiControls(bool* isOpen);

		void ResizeBounds(float width, float height);
		void SetProjection(float aspectRatio);
		inline void SetZoomLevel(float zoomlevel) { m_ZoomLevel = zoomlevel; RecalculateView(); }
		inline float GetZoomLevel() const { return m_ZoomLevel; }

		inline void SetRotation(float rotation) { m_Rotation = rotation; }
		inline float GetRotation() const { return m_Rotation; }

		inline OrthographicCamera& GetCamera() { return m_Camera; }

		inline OrthographicCameraBounds& GetBounds() { return m_Bounds; }

		inline bool ShouldUpdate() { return m_ShouldUpdate; }

	private:
		void RecalculateView();

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;

		OrthographicCameraBounds m_Bounds;
		OrthographicCamera m_Camera;
		bool m_DoRotate;

		bool m_ShouldUpdate = false;

		glm::vec3 m_Position = glm::vec3(0.0f);
		float m_Rotation = 0.0f;
		float m_CameraTranslationSpeed = 3.0f;
		float m_CameraRotationSpeed = 45.0f;
		float m_ScrollSpeed = 0.1f;
	};
}