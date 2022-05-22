#pragma once

#include <glm/glm.hpp>

namespace Acorn
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(glm::mat4 projection)
			: m_Projection(projection)
		{
		}
		virtual ~Camera() = default;

		inline const glm::mat4 &GetProjection() const { return m_Projection; }

	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};

	class OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top)
			: OrthographicCamera(left, right, bottom, top, -1.0f, 1.0f)
		{
		}
		OrthographicCamera(float left, float right, float bottom, float top, float zNear, float zFar);

		inline void SetProjection(float left, float right, float bottom, float top) { SetProjection(left, right, bottom, top, -1.0f, 1.0f); }
		void SetProjection(float left, float right, float bottom, float top, float zNear, float zFar);

		inline const glm::vec3 &GetPosition() const { return m_Position; }
		inline void SetPosition(const glm::vec3 &position)
		{
			m_Position = position;
			RecalculateViewMatrix();
		}

		inline float GetRotation() const { return m_Rotation; }
		inline void SetRotation(float rotation)
		{
			m_Rotation = glm::radians(rotation);
			RecalculateViewMatrix();
		}

		inline const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4 &GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		float m_Rotation = 0.0f;
	};
}