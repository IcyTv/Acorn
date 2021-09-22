#include "acpch.h"

#include "ShapeEditor.h"

#include <imgui.h>

namespace Acorn
{
	void ShapeEditor::Draw()
	{
		//https://github.com/ocornut/imgui/blob/6470681d87de83018aaf85e118a6f5ab4df65e67/imgui_demo.cpp#L7012
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(300, 300));
		ImGui::Begin("Shape Editor", &m_Visible);

		ImGui::BeginChild("##ShapeEditor");

		ImVec2 canvasP0 = ImGui::GetCursorScreenPos();
		ImVec2 canvasSz = ImGui::GetContentRegionAvail();

		ImVec2 canvasP1 = ImVec2(canvasP0.x + canvasSz.x, canvasP0.y + canvasSz.y);

		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(canvasP0, canvasP1, ImColor(50, 50, 50, 255));
		drawList->AddRect(canvasP0, canvasP1, ImColor(255, 255, 255, 255));

		//Catch interactions
		ImGui::InvisibleButton("canvas", canvasSz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		const bool isHovered = ImGui::IsItemHovered();
		const bool isActive = ImGui::IsItemActive(); //Held
		const ImVec2 origin(canvasP0.x + m_Scrolling.x, canvasP0.y + m_Scrolling.y);
		const ImVec2 mousePosInCanvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

		if (isHovered && !m_AddingLine && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_AddingLine = true;
			m_Points.push_back(glm::vec2{mousePosInCanvas.x, mousePosInCanvas.y});
			m_Points.push_back(glm::vec2{mousePosInCanvas.x, mousePosInCanvas.y});
		}
		if (m_AddingLine)
		{
			m_Points.back() = glm::vec2{mousePosInCanvas.x, mousePosInCanvas.y};
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
				m_AddingLine = false;
		}

		ImGui::EndChild();
		ImGui::End();
		ImGui::PopStyleVar();
	}

}