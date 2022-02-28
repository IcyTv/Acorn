/*
 * Copyright (c) 2020, Benoit Lormeau <blormeau@outlook.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "GenericLexer.h"
#include "CharacterTypes.h"
#include "Utf16View.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace Acorn::IDL
{
	// Consume a number of characters
	std::string_view GenericLexer::consume(size_t count)
	{
		if (count == 0)
			return {};

		size_t start = m_index;
		size_t length = std::min(count, m_input.length() - m_index);
		m_index += length;

		return m_input.substr(start, length);
	}

	// Consume the rest of the input
	std::string_view GenericLexer::consume_all()
	{
		if (is_eof())
			return {};

		auto rest = m_input.substr(m_index, m_input.length() - m_index);
		m_index = m_input.length();
		return rest;
	}

	// Consume until a new line is found
	std::string_view GenericLexer::consume_line()
	{
		size_t start = m_index;
		while (!is_eof() && peek() != '\r' && peek() != '\n')
			m_index++;
		size_t length = m_index - start;

		consume_specific('\r');
		consume_specific('\n');

		if (length == 0)
			return {};
		return m_input.substr(start, length);
	}

	// Consume and return characters until `stop` is peek'd
	std::string_view GenericLexer::consume_until(char stop)
	{
		size_t start = m_index;
		while (!is_eof() && peek() != stop)
			m_index++;
		size_t length = m_index - start;

		if (length == 0)
			return {};
		return m_input.substr(start, length);
	}

	// Consume and return characters until the string `stop` is found
	std::string_view GenericLexer::consume_until(const char* stop)
	{
		size_t start = m_index;
		while (!is_eof() && !next_is(stop))
			m_index++;
		size_t length = m_index - start;

		if (length == 0)
			return {};
		return m_input.substr(start, length);
	}

	// Consume and return characters until the string `stop` is found
	std::string_view GenericLexer::consume_until(std::string_view stop)
	{
		size_t start = m_index;
		while (!is_eof() && !next_is(stop))
			m_index++;
		size_t length = m_index - start;

		if (length == 0)
			return {};
		return m_input.substr(start, length);
	}

	/*
	 * Consume a string surrounded by single or double quotes. The returned
	 * std::string_view does not include the quotes. An escape character can be provided
	 * to capture the enclosing quotes. Please note that the escape character will
	 * still be in the resulting std::string_view
	 */
	std::string_view GenericLexer::consume_quoted_string(char escape_char)
	{
		if (!next_is(is_quote))
			return {};

		char quote_char = consume();
		size_t start = m_index;
		while (!is_eof())
		{
			if (next_is(escape_char))
				m_index++;
			else if (next_is(quote_char))
				break;
			m_index++;
		}
		size_t length = m_index - start;

		if (peek() != quote_char)
		{
			// Restore the index in case the string is unterstd::minated
			m_index = start - 1;
			return {};
		}

		// Ignore closing quote
		ignore();

		return m_input.substr(start, length);
	}

	std::string GenericLexer::consume_and_unescape_string(char escape_char)
	{
		auto view = consume_quoted_string(escape_char);
		if (view.empty())
			return {};

		std::stringstream builder;
		for (size_t i = 0; i < view.length(); ++i)
			builder << consume_escaped_character(escape_char);
		return builder.str();
	}

	auto GenericLexer::consume_escaped_code_point(bool combine_surrogate_pairs) -> uint32_t
	{
		// 	if (!consume_specific("\\u"sv))
		// 		return UnicodeEscapeError::MalformedUnicodeEscape;
		assert(consume_specific("\\u"sv) || !"Malformed unicode escape");

		if (next_is('{'))
			return decode_code_point();
		return decode_single_or_paired_surrogate(combine_surrogate_pairs);
	}

	auto GenericLexer::decode_code_point() -> uint32_t
	{
		bool starts_with_open_bracket = consume_specific('{');
		assert(starts_with_open_bracket || !"Does not start with open bracket");

		uint32_t code_point = 0;

		while (true)
		{
			// if (!next_is(is_ascii_hex_digit))
			// 	return UnicodeEscapeError::MalformedUnicodeEscape;
			assert(next_is(is_ascii_hex_digit) || !"Does not start with hex digit");

			auto new_code_point = (code_point << 4u) | parse_ascii_hex_digit(consume());
			// if (new_code_point < code_point)
			// 	return UnicodeEscapeError::UnicodeEscapeOverflow;
			assert(new_code_point >= code_point || !"Overflow");

			code_point = new_code_point;
			if (consume_specific('}'))
				break;
		}

		if (is_unicode(code_point))
			return code_point;
		// return UnicodeEscapeError::UnicodeEscapeOverflow;
		assert(false || !"Overflow");
	}

	auto GenericLexer::decode_single_or_paired_surrogate(bool combine_surrogate_pairs) -> uint32_t
	{
		constexpr size_t surrogate_length = 4;

		auto decode_one_surrogate = [&]() -> std::optional<uint16_t>
		{
			uint16_t surrogate = 0;

			for (size_t i = 0; i < surrogate_length; ++i)
			{
				if (!next_is(is_ascii_hex_digit))
					return {};

				surrogate = (surrogate << 4u) | parse_ascii_hex_digit(consume());
			}

			return surrogate;
		};

		auto high_surrogate = decode_one_surrogate();
		// if (!high_surrogate.has_value())
		// 	return UnicodeEscapeError::MalformedUnicodeEscape;
		assert(high_surrogate.has_value() || !"Malformed unicode escape");
		if (!Utf16View::is_high_surrogate(*high_surrogate))
			return *high_surrogate;
		if (!combine_surrogate_pairs || !consume_specific("\\u"sv))
			return *high_surrogate;

		auto low_surrogate = decode_one_surrogate();
		// if (!low_surrogate.has_value())
		// 	return UnicodeEscapeError::MalformedUnicodeEscape;
		assert(low_surrogate.has_value() || !"Malformed unicode escape");
		if (Utf16View::is_low_surrogate(*low_surrogate))
			return Utf16View::decode_surrogate_pair(*high_surrogate, *low_surrogate);

		retreat(6);
		return *high_surrogate;
	}
}