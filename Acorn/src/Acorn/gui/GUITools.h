#pragma once

#include "core/Core.h"
#include "renderer/Texture.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Acorn::GUI
{
	// NOTE: maybe we should inline these functions. It'll make this file less readable
	// and the compiler (at -O2 or -O3) should be able to optimize them out.
	void BeginTable(std::string_view id, float columnWidth = 100.0f);
	void EndTable();

	template <typename T>
	void Item(std::string_view label, T&& func)
	{
		// TODO I want the text to be cut off by the ellipsis if it's too long
		ImGui::Text(label.data());
		ImGui::NextColumn();
		func();
		ImGui::NextColumn();
	}

	// Convinience functions that (kind of) mirror ImGUI
	template <typename... Args>
	void Text(std::string_view text, Args&&... args)
	{
		ImGui::Text(text.data(), std::forward<Args>(args)...);
		ImGui::NextColumn();
	}

	/**
	 * @brief ColorEdit for glm::vec4/3
	 *
	 * @tparam T either glm::vec4 or glm::vec3
	 * @param label Should be prefixed with '##' so the ColorEdit label doesn't show up in the table
	 * @param vec The vector to edit
	 */
	template <typename T>
	void ColorEdit(std::string_view label, T& vec);

	template <typename T>
	bool Drag(std::string_view label, T& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, std::string_view format = "%f");

	inline void Checkbox(std::string_view label, bool& value)
	{
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		ImGui::Checkbox(label.data(), &value);
		ImGui::NextColumn();
	}

	template <typename E>
	bool Combo(std::string_view label, E& value, const std::unordered_map<E, std::string_view>& names)
	{
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();

		bool changed = false;
		if (ImGui::BeginCombo(label.data(), names.at(value).data()))
		{
			for (auto [key, val] : names)
			{
				bool isSelected = (key == value);
				if (ImGui::Selectable(val.data(), isSelected))
				{
					value = key;
					changed = true;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::NextColumn();

		return changed;
	}

	inline void ImageButton(std::string_view label, Ref<Texture2d> texture, const glm::vec2& size = glm::vec2{25.0f})
	{
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		ImGui::ImageButton((void*)(intptr_t)texture->GetRendererId(), ImVec2{size.x, size.y});
		ImGui::NextColumn();
	}

	inline void TableButton(std::string_view label, const glm::vec2& size = glm::vec2{25.0f})
	{
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		ImGui::Button("", ImVec2{size.x, size.y});
		ImGui::NextColumn();
	}

}