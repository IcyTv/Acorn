#include "acpch.h"

#include "EditorCamera.h"

#include "input/Input.h"
#include "input/KeyCodes.h"
#include "input/MouseButtonCodes.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

constexpr float MAX_PAN_SPEED = 2.4f;
constexpr float PAN_SQUARE_FACTOR = 0.0366f;
constexpr float PAN_REDUCTION_FACTOR = 0.1778f;
constexpr float PAN_ADD_FACTOR = 0.3021f;
constexpr float MOUSE_DELTA_FACTOR = 0.003f;
constexpr float MOUSE_SCROLL_FACTOR = 0.1f;

constexpr float ZOOM_FACTOR = 0.2f;
constexpr float ZOOM_MIN_DISTANCE = 0.0f;
constexpr float ZOOM_MAX_SPEED = 100.0f;

constexpr bool LOCK_CAMERA_ROTATION = false;

namespace Acorn
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_Fov(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		if (LOCK_CAMERA_ROTATION)
		{
			m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		}
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, MAX_PAN_SPEED); // max = 2.4f
		float xFactor = PAN_SQUARE_FACTOR * (x * x) - PAN_REDUCTION_FACTOR * x + PAN_ADD_FACTOR;

		float y = std::min(m_ViewportHeight / 1000.0f, MAX_PAN_SPEED); // max = 2.4f
		float yFactor = PAN_SQUARE_FACTOR * (y * y) - PAN_REDUCTION_FACTOR * y + PAN_ADD_FACTOR;

		return {xFactor, yFactor};
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * ZOOM_FACTOR;
		distance = std::max(distance, ZOOM_MIN_DISTANCE);
		float speed = distance * distance;
		speed = std::min(speed, ZOOM_MAX_SPEED); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mouse{Input::GetMouseX(), Input::GetMouseY()};
			glm::vec2 delta = (mouse - m_InitialMousePosition) * MOUSE_DELTA_FACTOR;
			m_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(AC_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(AC_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(AC_MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(EditorCamera::OnMouseScroll));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(EditorCamera::OnMouseButtonDown));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetOffsetY() * MOUSE_SCROLL_FACTOR;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	bool EditorCamera::OnMouseButtonDown(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == AC_MOUSE_BUTTON_LEFT)
			m_InitialMousePosition = {Input::GetMouseX(), Input::GetMouseY()};
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void EditorCamera::SetFocalPointDistance(const glm::vec3& focalPoint, float distance)
	{
		m_FocalPoint = focalPoint;
		m_Distance = distance;
		UpdateView();
	}
}