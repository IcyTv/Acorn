#include "acpch.h"

#include "utils/MathUtils.h"
#include <glm/matrix.hpp>

namespace Acorn
{

}
namespace Acorn
{
	namespace Utils
	{
		glm::vec3 ToWorldCoords(const glm::vec2& position, const Camera& camera, const glm::mat4& transform)
		{
			glm::vec4 screenPos = glm::vec4(position, 0.0f, 1.0f);
			glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

			glm::vec4 worldPos = viewProj * screenPos;

			return glm::vec3(worldPos) / worldPos.w;
		}

	}
}