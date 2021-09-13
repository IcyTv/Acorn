
#include <Acorn.h>

#include "spdlog/details/null_mutex.h"
#include <imgui.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <mutex>

namespace Acorn
{
	class LogPanel : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
	{
	public:
		LogPanel();
		~LogPanel() = default;

		void Clear();
		void AddLog(const char* fmt, ...) IM_FMTARGS(2);
		void OnImGuiRender();

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override;
		void flush_() override;

	private:
		ImGuiTextBuffer m_Buffer;
		ImGuiTextFilter m_Filter;
		ImVector<int> m_LineOffsets;
		bool m_AutoScroll;

		bool m_IsOpen = true;
	};
}