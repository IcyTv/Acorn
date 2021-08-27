#include "acpch.h"

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Acorn
{

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, zNear, zFar)), m_Position(glm::vec3(0.0f)), m_ViewMatrix(glm::mat4(1.0f))
	{
		AC_PROFILE_FUNCTION();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		AC_PROFILE_FUNCTION();

		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, zNear, zFar);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		AC_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0), m_Position);
		transform = glm::rotate(transform, m_Rotation, {0, 0, 1});

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}