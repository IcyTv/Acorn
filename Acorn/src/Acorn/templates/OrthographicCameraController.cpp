#include "acpch.h"

#include "input/Input.h"
#include "input/KeyCodes.h"
#include "templates/OrthographicCameraController.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Acorn
{

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_DoRotate(rotation), m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top), m_AspectRatio(aspectRatio), m_Bounds({-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel})
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(AC_KEY_A) || Input::IsKeyPressed(AC_KEY_LEFT))
		{
			m_Position.x -= cos(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
			m_Position.y -= sin(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(AC_KEY_D) || Input::IsKeyPressed(AC_KEY_RIGHT))
		{
			m_Position.x += cos(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
			m_Position.y += sin(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(AC_KEY_W) || Input::IsKeyPressed(AC_KEY_UP))
		{
			m_Position.x += -sin(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
			m_Position.y += cos(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(AC_KEY_S) || Input::IsKeyPressed(AC_KEY_DOWN))
		{
			m_Position.x -= -sin(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
			m_Position.y -= cos(glm::radians(m_Rotation)) * m_CameraTranslationSpeed * ts;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(AC_KEY_Q))
				m_Rotation += m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(AC_KEY_E))
				m_Rotation -= m_CameraRotationSpeed * ts;

			if (m_Rotation > 180.0f)
				m_Rotation -= 360.0f;
			else if (m_Rotation <= -180.0f)
				m_Rotation += 360.0f;

			m_Camera.SetRotation(m_Rotation);
		}

		m_Camera.SetPosition(m_Position);

		m_CameraTranslationSpeed = m_ZoomLevel;

		m_ShouldUpdate = false;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	void OrthographicCameraController::ImGuiControls(bool* isOpen)
	{
		if (*isOpen)
		{
			if (ImGui::Begin("Orthographic Camera Controls", isOpen))
			{
				if (ImGui::DragFloat3("Position", glm::value_ptr(m_Position), 0.1f))
				{
					m_ShouldUpdate = true;
				}
				if (
					ImGui::DragFloat("Rotation", &m_Rotation, 1.0f))
				{
					m_ShouldUpdate = true;
				}
				ImGui::DragFloat("Movement Speed", &m_CameraTranslationSpeed, 0.1f);
				ImGui::DragFloat("Rotation Speed", &m_CameraRotationSpeed, 0.5f);
				ImGui::DragFloat("Zoom Speed", &m_ScrollSpeed, 0.01f);
				ImGui::End();
			}
		}
	}

	void OrthographicCameraController::ResizeBounds(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Bounds = {-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel};
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	void OrthographicCameraController::SetProjection(float aspectRatio)
	{
	}

	void OrthographicCameraController::RecalculateView()
	{
		m_Bounds = {-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel};
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetOffsetY() * m_ScrollSpeed;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		RecalculateView();
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		RecalculateView();
		return false;
	}

}