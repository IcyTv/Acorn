#pragma once

#include "core/Core.h"

#include <vector>

#include <glm/glm.hpp>

namespace Acorn
{
	class ShapeEditor
	{
	public:
		void Draw();

	private:
		bool m_Visible = false;

		bool m_EnableGrid = true;
		bool m_EnableContextMenu = true;
		bool m_AddingLine = false;
		glm::vec2 m_Scrolling = glm::vec2{0.0f};

		std::vector<glm::vec2> m_Points;
	};
}