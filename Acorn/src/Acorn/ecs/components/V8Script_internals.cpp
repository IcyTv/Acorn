#include "acpch.h"

#include "ecs/components/V8Script_internals.h"

#include "ecs/components/V8Script.h"

#include <glm/glm.hpp>

namespace Acorn::internals
{
	glm::vec2 Utils::TranslateMousePosition(glm::vec2 position)
	{
		glm::mat4 ViewProjectionMatrix = V8Engine::instance().GetData().PrimaryCameraViewProjectionMatrix;
		glm::vec4 ScreenSpacePosition = ViewProjectionMatrix * glm::vec4(position, 0.0f, 1.0f);
		glm::vec2 NormalizedPosition = glm::vec2(ScreenSpacePosition.x / ScreenSpacePosition.w, ScreenSpacePosition.y / ScreenSpacePosition.w);
		return NormalizedPosition;
	}

}