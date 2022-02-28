/*
 * Copyright (c) 2020, Benoit Lormeau <blormeau@outlook.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

using namespace std::literals;

template <class P>
concept HasLength = requires(P p)
{
	p.length();
};

namespace Acorn::IDL
{

	class GenericLexer
	{
	public:
		constexpr explicit GenericLexer(std::string_view input)
			: m_input(input)
		{
		}

		constexpr size_t tell() const { return m_index; }
		constexpr size_t tell_remaining() const { return m_input.length() - m_index; }

		std::string_view remaining() const { return m_input.substr(m_index); }

		constexpr bool is_eof() const { return m_index >= m_input.length(); }

		constexpr char peek(size_t offset = 0) const
		{
			return (m_index + offset < m_input.length()) ? m_input[m_index + offset] : '\0';
		}

		constexpr bool next_is(char expected) const
		{
			return peek() == expected;
		}

		constexpr bool next_is(std::string_view expected) const
		{
			for (size_t i = 0; i < expected.length(); ++i)
				if (peek(i) != expected[i])
					return false;
			return true;
		}

		constexpr bool next_is(const char* expected) const
		{
			for (size_t i = 0; expected[i] != '\0'; ++i)
				if (peek(i) != expected[i])
					return false;
			return true;
		}

		constexpr void retreat()
		{
			assert(m_index > 0);
			--m_index;
		}

		constexpr void retreat(size_t count)
		{
			assert(m_index >= count);
			m_index -= count;
		}

		constexpr char consume()
		{
			assert(!is_eof());
			return m_input[m_index++];
		}

		template <typename T>
		constexpr bool consume_specific(const T& next)
		{
			if (!next_is(next))
				return false;

			if constexpr (HasLength<T>)
			{
				ignore(next.length());
			}
			else
			{
				ignore(sizeof(next));
			}
			return true;
		}

		bool consume_specific(const std::string& next)
		{
			return consume_specific(std::string_view{next});
		}

		constexpr bool consume_specific(const char* next)
		{
			return consume_specific(std::string_view{next});
		}

		constexpr char consume_escaped_character(char escape_char = '\\', std::string_view escape_map = "n\nr\rt\tb\bf\f"sv)
		{
			if (!consume_specific(escape_char))
				return consume();

			auto c = consume();

			for (size_t i = 0; i < escape_map.length(); i += 2)
			{
				if (c == escape_map[i])
					return escape_map[i + 1];
			}

			return c;
		}

		std::string_view consume(size_t count);
		std::string_view consume_all();
		std::string_view consume_line();
		std::string_view consume_until(char);
		std::string_view consume_until(const char*);
		std::string_view consume_until(std::string_view);
		std::string_view consume_quoted_string(char escape_char = 0);
		std::string consume_and_unescape_string(char escape_char = '\\');

		enum class UnicodeEscapeError
		{
			MalformedUnicodeEscape,
			UnicodeEscapeOverflow,
		};

		uint32_t consume_escaped_code_point(bool combine_surrogate_pairs = true);

		constexpr void ignore(size_t count = 1)
		{
			count = std::min(count, m_input.length() - m_index);
			m_index += count;
		}

		constexpr void ignore_until(char stop)
		{
			while (!is_eof() && peek() != stop)
			{
				++m_index;
			}
			ignore();
		}

		constexpr void ignore_until(const char* stop)
		{
			while (!is_eof() && !next_is(stop))
			{
				++m_index;
			}
			ignore(__builtin_strlen(stop));
		}

		/*
		 * Conditions are used to match arbitrary characters. You can use lambdas,
		 * ctype functions, or is_any_of() and its derivatives (see below).
		 * A few examples:
		 *   - `if (lexer.next_is(isdigit))`
		 *   - `auto name = lexer.consume_while([](char c) { return isalnum(c) || c == '_'; });`
		 *   - `lexer.ignore_until(is_any_of("<^>"));`
		 */

		// Test the next character against a Condition
		template <typename TPredicate>
		constexpr bool next_is(TPredicate pred) const
		{
			return pred(peek());
		}

		// Consume and return characters while `pred` returns true
		template <typename TPredicate>
		std::string_view consume_while(TPredicate pred)
		{
			size_t start = m_index;
			while (!is_eof() && pred(peek()))
				++m_index;
			size_t length = m_index - start;

			if (length == 0)
				return {};
			return m_input.substr(start, length);
		}

		// Consume and return characters until `pred` return true
		template <typename TPredicate>
		std::string_view consume_until(TPredicate pred)
		{
			size_t start = m_index;
			// bool test = !pred(peek());
			// bool eof = is_eof();
			while (!is_eof() && !pred(peek()))
			{
				++m_index;
			}
			size_t length = m_index - start;

			if (length == 0)
				return {};
			return m_input.substr(start, length);
		}

		// Ignore characters while `pred` returns true
		template <typename TPredicate>
		constexpr void ignore_while(TPredicate pred)
		{
			while (!is_eof() && pred(peek()))
				++m_index;
		}

		// Ignore characters until `pred` return true
		// We don't skip the stop character as it may not be a unique value
		template <typename TPredicate>
		constexpr void ignore_until(TPredicate pred)
		{
			while (!is_eof() && !pred(peek()))
				++m_index;
		}

	protected:
		std::string_view m_input;
		size_t m_index{0};

	private:
		uint32_t decode_code_point();
		uint32_t decode_single_or_paired_surrogate(bool combine_surrogate_pairs);
	};

	constexpr auto is_any_of(std::string_view values)
	{
		return [values](auto c)
		{ return values.find(c) != std::string::npos; };
	}

	constexpr auto is_path_separator = is_any_of("/\\");
	constexpr auto is_quote = is_any_of("'\"");
}