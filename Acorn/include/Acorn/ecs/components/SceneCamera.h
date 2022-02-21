#pragma once

#include "core/Core.h"
#include "renderer/Camera.h"

#include <magic_enum.hpp>

#define GET_SET(name)                                   \
	inline float Get##name() const { return m_##name; } \
	inline void Set##name(float val)                    \
	{                                                   \
		m_##name = val;                                 \
		RecalculateProjectionMatrix();                  \
	}

namespace Acorn
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType
		{
			Perspective = 0,
			Orthographic = 1,
		};

	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float fov, float nearClip, float farClip);
		void SetViewportSize(uint32_t width, uint32_t height);

		GET_SET(OrthographicSize);
		GET_SET(OrthographicNearClip);
		GET_SET(OrthographicFarClip);

		GET_SET(PerspectiveFov);
		GET_SET(PerspectiveNearClip);
		GET_SET(PerspectiveFarClip);

		inline ProjectionType GetProjectionType() const { return m_ProjectionType; }
		inline void SetProjectionType(int i) { SetProjectionType((ProjectionType)i); }
		inline void SetProjectionType(ProjectionType type)
		{
			m_ProjectionType = type;
			RecalculateProjectionMatrix();
		}

		inline float GetAspectRatio() const { return m_AspectRatio; }

	private:
		void RecalculateProjectionMatrix();

	private:
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNearClip = -1.0f, m_OrthographicFarClip = 1.0f;

		float m_PerspectiveFov = glm::radians(60.0f);
		float m_PerspectiveNearClip = 0.01f, m_PerspectiveFarClip = 1000.0f;

		float m_AspectRatio = 1.0f;

		ProjectionType m_ProjectionType = ProjectionType::Orthographic;
	};
}