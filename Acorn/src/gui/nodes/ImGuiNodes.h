#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>
#include <imgui_internal.h>
#include "ImNodesEz.h"

namespace Acorn::gui::nodes
{
	struct Connection
	{
		/// `id` that was passed to BeginNode() of input node.
		void* InputNode = nullptr;
		/// Descriptor of input slot.
		const char* InputSlot = nullptr;
		/// `id` that was passed to BeginNode() of output node.
		void* OutputNode = nullptr;
		/// Descriptor of output slot.
		const char* OutputSlot = nullptr;

		bool operator==(const Connection& other) const
		{
			return InputNode == other.InputNode &&
				InputSlot == other.InputSlot &&
				OutputNode == other.OutputNode &&
				OutputSlot == other.OutputSlot;
		}

		bool operator!=(const Connection& other) const
		{
			return !operator ==(other);
		}
	};

	struct Node
	{
		const char* Title = nullptr;
		bool Selected = false;
		ImVec2 Pos{};
		std::vector<Connection> Connections{};
		std::vector<ImNodes::Ez::SlotInfo> InputSlots{};
		std::vector<ImNodes::Ez::SlotInfo> OutputSlots{};

		explicit Node(const char* title,
			const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
			const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
		{
			Title = title;
			InputSlots = input_slots;
			OutputSlots = output_slots;
		}

		/// Deletes connection from this node.
		void DeleteConnection(const Connection& connection)
		{
			for (auto it = Connections.begin(); it != Connections.end(); ++it)
			{
				if (connection == *it)
				{
					Connections.erase(it);
					break;
				}
			}
		}
	};

	typedef std::unordered_map<std::string, Node* (*)()> NodeMap;

	class NodeNetwork
	{
	public:
		NodeNetwork(NodeMap nodeMap)
			: m_NodeMap(nodeMap)
		{
		}

		void Draw();

	private:

	private:
		ImNodes::CanvasState m_Canvas;

		std::vector<Node*> m_Nodes;
		NodeMap m_NodeMap;
	};
}