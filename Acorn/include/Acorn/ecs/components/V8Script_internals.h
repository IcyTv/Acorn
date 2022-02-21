#pragma once

#include "core/Core.h"
#include <glm/fwd.hpp>

namespace Acorn::internals
{
	class Utils
	{
	public:
		static glm::vec2 TranslateMousePosition(glm::vec2 position);
	};
}