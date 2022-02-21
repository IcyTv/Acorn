#pragma once

#include "core/Core.h"
#include "ecs/components/SceneCamera.h"
#include "renderer/Camera.h"

#include <glm/glm.hpp>

namespace Acorn
{
	namespace Utils
	{
		/* Convert a glm::vec2 screen position to world coordinates */
		glm::vec3 ToWorldCoords(const glm::vec2& position, const Camera& camera, const glm::mat4& transform);
	}
}