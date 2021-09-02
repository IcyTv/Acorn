#include "acpch.h"

#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Acorn
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjectionMatrix();
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNearClip = nearClip;
		m_OrthographicFarClip = farClip;

		RecalculateProjectionMatrix();
	}

	void SceneCamera::SetPerspective(float fov, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFov = fov;
		m_PerspectiveNearClip = nearClip;
		m_PerspectiveFarClip = farClip;

		RecalculateProjectionMatrix();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		AC_CORE_ASSERT(width > 0 && height > 0, "Invalid viewport size!");
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjectionMatrix();
	}

	void SceneCamera::RecalculateProjectionMatrix()
	{
		if (m_ProjectionType == ProjectionType::Orthographic)
		{
			float orthoLeft = -0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoRight = 0.5f * m_OrthographicSize * m_AspectRatio;
			float orthoBottom = -0.5f * m_OrthographicSize;
			float orthoTop = 0.5f * m_OrthographicSize;

			m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNearClip, m_OrthographicFarClip);
		}
		else
		{
			m_Projection = glm::perspective(m_PerspectiveFov, m_AspectRatio, m_PerspectiveNearClip, m_PerspectiveFarClip);
		}
	}
}