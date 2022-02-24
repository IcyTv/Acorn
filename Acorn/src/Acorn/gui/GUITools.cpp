#include "acpch.h"

#include "gui/GUITools.h"

#include <imgui.h>

namespace Acorn::GUI
{
	void BeginTable(std::string_view id, float columnWidth)
	{
		ImGui::PushID(id.data());
		ImGui::Columns(2);
	}

	void EndTable()
	{
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template <typename T>
	void ColorEdit(std::string_view label, T& vec)
	{
		static_assert(false, "ColorEdit for type not implemented");
	}

	template <>
	void ColorEdit(std::string_view label, glm::vec3& vec)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "ColorEdit label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		ImGui::ColorEdit3(label.data(), glm::value_ptr(vec));
		ImGui::NextColumn();
	}

	template <>
	void ColorEdit(std::string_view label, glm::vec4& vec)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "ColorEdit label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		ImGui::ColorEdit4(label.data(), glm::value_ptr(vec));
		ImGui::NextColumn();
	}

	template <typename T>
	bool Drag(std::string_view label, T& value, float speed, float min, float max, std::string_view format)
	{
		static_assert(false, "Drag for type not implemented");
	}

	template <>
	bool Drag(std::string_view label, float& value, float speed, float min, float max, std::string_view format)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "Drag label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		bool changed = ImGui::DragFloat(label.data(), &value, speed, min, max, format.data());
		ImGui::NextColumn();
		return changed;
	}

	template <>
	bool Drag(std::string_view label, int& value, float speed, float min, float max, std::string_view format)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "Drag label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		bool changed = ImGui::DragInt(label.data(), &value, speed, static_cast<int>(min), static_cast<int>(max), format.data());
		ImGui::NextColumn();
		return changed;
	}

	template <>
	bool Drag(std::string_view label, glm::vec2& value, float speed, float min, float max, std::string_view format)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "Drag label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		bool changed = ImGui::DragFloat2(label.data(), glm::value_ptr(value), speed, min, max, format.data());
		ImGui::NextColumn();
		return changed;
	}

	template <>
	bool Drag(std::string_view label, glm::vec3& value, float speed, float min, float max, std::string_view format)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "Drag label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		bool changed = ImGui::DragFloat3(label.data(), glm::value_ptr(value), speed, min, max, format.data());
		ImGui::NextColumn();
		return changed;
	}

	template <>
	bool Drag(std::string_view label, glm::vec4& value, float speed, float min, float max, std::string_view format)
	{
		AC_CORE_ASSERT(label.starts_with("##"), "Drag label must be prefixed with '##'");
		ImGui::Text(label.data() + 2);
		ImGui::NextColumn();
		bool changed = ImGui::DragFloat4(label.data(), glm::value_ptr(value), speed, min, max, format.data());
		ImGui::NextColumn();
		return changed;
	}
} // namespace Acorn
