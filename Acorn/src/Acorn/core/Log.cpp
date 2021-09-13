#include "acpch.h"

#include "Log.h"
#include "spdlog/pattern_formatter.h"

namespace Acorn
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		//TODO figure out a good way to use these [%10!s:%#] (%=7!!)
		spdlog::set_pattern("%^[%=8n][%T]: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("Acorn");
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->set_pattern("%^[%=8n][%T][%l]: %v%$");

		s_ClientLogger = spdlog::stdout_color_mt("Your App");
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->set_pattern("%^[%=8n][%T][%l]: %v%$");
	}

	std::shared_ptr<spdlog::logger> &Log::GetCoreLogger()
	{
		return s_CoreLogger;
	}

	std::shared_ptr<spdlog::logger> &Log::GetClientLogger()
	{
		return s_ClientLogger;
	}

	void Log::AddSink(const spdlog::sink_ptr &sink)
	{
		s_CoreLogger->sinks().push_back(sink);
		s_ClientLogger->sinks().push_back(sink);
	}

}