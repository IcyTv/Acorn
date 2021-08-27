#pragma once

DISABLE_ALL_WARNINGS_BEGIN
#include <imterm/terminal.hpp>
#include <imterm/terminal_helpers.hpp>
DISABLE_ALL_WARNINGS_END

namespace Acorn
{

	struct CustomCommandStruct
	{};
	class TerminalCommands: public ImTerm::basic_spdlog_terminal_helper<TerminalCommands, CustomCommandStruct, misc::no_mutex>
	{
	public:
		static std::vector<std::string> no_completion(argument_type&) { return {}; }

		// clears the logging screen. argument_type is aliased in ImTerm::basic_terminal_helper
		static void clear(argument_type& arg) {
			arg.term.clear();
		}

		// prints the text passed as parameter to the logging screen. argument_type is aliased in ImTerm::basic_terminal_helper
		static void echo(argument_type& arg) {
			if (arg.command_line.size() < 2) {
				return;
			}
			std::string str = std::move(arg.command_line[1]);
			for (auto it = std::next(arg.command_line.begin(), 2); it != arg.command_line.end(); ++it) {
				str += " " + std::move(*it);
			}
			ImTerm::message msg;
			msg.value = std::move(str);
			msg.color_beg = msg.color_end = 0; // color is disabled when color_beg == color_end
			// other parameters are ignored
			arg.term.add_message(std::move(msg));
		}

		TerminalCommands() {
			add_command_({ "clear", "clear the screen", clear, no_completion });
			add_command_({ "echo", "echoes your text", echo, no_completion });
		}
	};
}