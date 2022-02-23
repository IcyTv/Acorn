#include "LogPanel.h"

namespace Acorn
{
	LogPanel::LogPanel()
	{
		m_AutoScroll = true;
		Clear();
	}

	void LogPanel::Clear()
	{
		m_Buffer.clear();
		m_LineOffsets.clear();
		m_LineOffsets.push_back(0);
	}

	void IM_FMTARGS(2) LogPanel::AddLog(const char* fmt, ...)
	{
		int old_size = m_Buffer.size();
		va_list args;
		va_start(args, fmt);
		m_Buffer.appendfv(fmt, args);
		va_end(args);
		for (int new_size = m_Buffer.size(); old_size < new_size; old_size++)
			if (m_Buffer[old_size] == '\n')
				m_LineOffsets.push_back(old_size + 1);
	}

	void LogPanel::OnImGuiRender()
	{
		if (!ImGui::Begin("Log", &m_IsOpen))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");

		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		m_Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = m_Buffer.begin();
		const char* buf_end = m_Buffer.end();
		if (m_Filter.IsActive())
		{
			for (int line_no = 0; line_no < m_LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + m_LineOffsets[line_no];
				const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
				if (m_Filter.PassFilter(line_start, line_end))
				{
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(m_LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + m_LineOffsets[line_no];
					const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}

	void LogPanel::sink_it_(const spdlog::details::log_msg& msg)
	{
		spdlog::memory_buf_t formatted;
		spdlog::sinks::base_sink<spdlog::details::null_mutex>::formatter_->format(msg, formatted);
		AddLog("%s", fmt::to_string(formatted).c_str());
	}

	void LogPanel::flush_()
	{
	}
}