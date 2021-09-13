
#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

namespace Acorn::Math
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	glm::vec2 WorldToImGui(const glm::vec3& worldPos, const glm::mat4& viewProjection, const glm::vec2& position, const glm::vec2& size);

}