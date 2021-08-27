#pragma once

#include "Core.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

#include "gui/terminal/Terminal.h"

#include <glm/gtx/io.hpp>

namespace Acorn {
	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();
		inline static void SetTerminal(std::shared_ptr<TerminalCommands> terminal) {
			s_CoreLogger->sinks().push_back(terminal);
			s_ClientLogger->sinks().push_back(terminal);
		}
		inline static void RemoveTerminal(std::shared_ptr<TerminalCommands> terminal)
		{
			auto coreSinks = s_CoreLogger->sinks();
			auto coreIt = std::find(coreSinks.begin(), coreSinks.end(), terminal);
			if (coreIt != coreSinks.end())
				coreSinks.erase(coreIt);

			auto clientSinks = s_ClientLogger->sinks();
			auto clientIt = std::find(clientSinks.begin(), clientSinks.end(), terminal);
			if (clientIt != clientSinks.end())
				clientSinks.erase(clientIt);
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

//Core Log Macros
#define AC_CORE_FATAL(...) SPDLOG_LOGGER_CRITICAL(::Acorn::Log::GetCoreLogger(), __VA_ARGS__)
#define AC_CORE_ERROR(...) SPDLOG_LOGGER_ERROR(::Acorn::Log::GetCoreLogger(), __VA_ARGS__)
#define AC_CORE_WARN(...)  SPDLOG_LOGGER_WARN(::Acorn::Log::GetCoreLogger(), __VA_ARGS__)
#define AC_CORE_INFO(...)  SPDLOG_LOGGER_INFO(::Acorn::Log::GetCoreLogger(), __VA_ARGS__)
#define AC_CORE_TRACE(...) SPDLOG_LOGGER_TRACE(::Acorn::Log::GetCoreLogger(), __VA_ARGS__)

//Client Log Macros
#define AC_FATAL(...) SPDLOG_LOGGER_CRITICAL(::Acorn::Log::GetClientLogger(), __VA_ARGS__)
#define AC_ERROR(...) SPDLOG_LOGGER_ERROR(::Acorn::Log::GetClientLogger(), __VA_ARGS__)
#define AC_WARN(...)  SPDLOG_LOGGER_WARN(::Acorn::Log::GetClientLogger(), __VA_ARGS__)
#define AC_INFO(...)  SPDLOG_LOGGER_INFO(::Acorn::Log::GetClientLogger(), __VA_ARGS__)
#define AC_TRACE(...) SPDLOG_LOGGER_TRACE(::Acorn::Log::GetClientLogger(), __VA_ARGS__)