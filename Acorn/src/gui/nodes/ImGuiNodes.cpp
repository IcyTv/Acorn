#include "acpch.h"

#include "ImGuiNodes.h"

namespace Acorn::gui::nodes
{

	void NodeNetwork::Draw()
	{
		ImNodes::BeginCanvas(&m_Canvas);

		for (auto it = m_Nodes.begin(); it != m_Nodes.end();)
		{
			Node* node = *it;

			if (ImNodes::Ez::BeginNode(node, node->Title, &node->Pos, &node->Selected))
			{
				ImNodes::Ez::InputSlots(node->InputSlots.data(), (int)node->InputSlots.size());

				ImGui::Text("Node Content %s", node->Title);

				ImNodes::Ez::OutputSlots(node->OutputSlots.data(), (int)node->OutputSlots.size());

				//Store new connections when they are created
				Connection new_connection;
				if (ImNodes::GetNewConnection(&new_connection.InputNode, &new_connection.InputSlot, &new_connection.OutputNode, &new_connection.OutputSlot))
				{
					((Node*)new_connection.InputNode)->Connections.push_back(new_connection);
					((Node*)new_connection.OutputNode)->Connections.push_back(new_connection);
				}

				for (const Connection& connection : node->Connections)
				{
					if(connection.OutputNode != node)
						continue;

					if (!ImNodes::Connection(connection.InputNode, connection.InputSlot, connection.OutputNode, connection.OutputSlot))
					{
						((Node*)connection.InputNode)->DeleteConnection(connection);
						((Node*)connection.OutputNode)->DeleteConnection(connection);
					}
				}
			}

			ImNodes::Ez::EndNode();

			if (node->Selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete))
			{
				// Deletion order is critical: first we delete connections to us
				for (auto& connection : node->Connections)
				{
					if (connection.OutputNode == node)
					{
						((Node*)connection.InputNode)->DeleteConnection(connection);
					}
					else
					{
						((Node*)connection.OutputNode)->DeleteConnection(connection);
					}
				}
				// Then we delete our own connections, so we don't corrupt the list
				node->Connections.clear();

				delete node;
				it = m_Nodes.erase(it);
			}
			else
				++it;
		}

		if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
		{
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
			ImGui::OpenPopup("NodesContextMenu");
		}

		if (ImGui::BeginPopup("NodesContextMenu"))
		{
			for (const auto& desc : m_NodeMap)
			{
				if (ImGui::MenuItem(desc.first.c_str()))
				{
					m_Nodes.push_back(desc.second());
					ImNodes::AutoPositionNode(m_Nodes.back());
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Reset Zoom"))
				m_Canvas.Zoom = 1;

			if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImNodes::EndCanvas();
	}

}