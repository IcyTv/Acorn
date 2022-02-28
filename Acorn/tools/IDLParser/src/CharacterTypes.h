/*
 * Copyright (c) 2021, Max Wipfli <mail@maxwipfli.ch>
 * Copyright (c) 2022, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <array>
#include <cassert>
// NOTE: For a quick reference for most of this, see https://www.cplusplus.com/reference/cctype/ and https://infra.spec.whatwg.org/#code-points.
// NOTE: To avoid ambiguity when including this header, all methods contains names should contain "ascii" or "unicode".

namespace Acorn::IDL
{

	constexpr bool is_ascii(uint32_t code_point)
	{
		return code_point < 0x80;
	}

	constexpr bool is_ascii_digit(uint32_t code_point)
	{
		return code_point >= '0' && code_point <= '9';
	}

	constexpr bool is_ascii_upper_alpha(uint32_t code_point)
	{
		return (code_point >= 'A' && code_point <= 'Z');
	}

	constexpr bool is_ascii_lower_alpha(uint32_t code_point)
	{
		return (code_point >= 'a' && code_point <= 'z');
	}

	constexpr bool is_ascii_alpha(uint32_t code_point)
	{
		return is_ascii_lower_alpha(code_point) || is_ascii_upper_alpha(code_point);
	}

	constexpr bool is_ascii_alphanumeric(uint32_t code_point)
	{
		return is_ascii_alpha(code_point) || is_ascii_digit(code_point);
	}

	constexpr bool is_ascii_binary_digit(uint32_t code_point)
	{
		return code_point == '0' || code_point == '1';
	}

	constexpr bool is_ascii_octal_digit(uint32_t code_point)
	{
		return code_point >= '0' && code_point <= '7';
	}

	constexpr bool is_ascii_hex_digit(uint32_t code_point)
	{
		return is_ascii_digit(code_point) || (code_point >= 'A' && code_point <= 'F') || (code_point >= 'a' && code_point <= 'f');
	}

	constexpr bool is_ascii_blank(uint32_t code_point)
	{
		return code_point == '\t' || code_point == ' ';
	}

	constexpr bool is_ascii_space(uint32_t code_point)
	{
		return code_point == ' ' || code_point == '\t' || code_point == '\n' || code_point == '\v' || code_point == '\f' || code_point == '\r';
	}

	constexpr bool is_ascii_punctuation(uint32_t code_point)
	{
		return (code_point >= 0x21 && code_point <= 0x2F) || (code_point >= 0x3A && code_point <= 0x40) || (code_point >= 0x5B && code_point <= 0x60) || (code_point >= 0x7B && code_point <= 0x7E);
	}

	constexpr bool is_ascii_graphical(uint32_t code_point)
	{
		return code_point >= 0x21 && code_point <= 0x7E;
	}

	constexpr bool is_ascii_printable(uint32_t code_point)
	{
		return code_point >= 0x20 && code_point <= 0x7E;
	}

	constexpr bool is_ascii_c0_control(uint32_t code_point)
	{
		return code_point < 0x20;
	}

	constexpr bool is_ascii_control(uint32_t code_point)
	{
		return is_ascii_c0_control(code_point) || code_point == 0x7F;
	}

	constexpr bool is_unicode(uint32_t code_point)
	{
		return code_point <= 0x10FFFF;
	}

	constexpr bool is_unicode_control(uint32_t code_point)
	{
		return is_ascii_c0_control(code_point) || (code_point >= 0x7E && code_point <= 0x9F);
	}

	constexpr bool is_unicode_surrogate(uint32_t code_point)
	{
		return code_point >= 0xD800 && code_point <= 0xDFFF;
	}

	constexpr bool is_unicode_scalar_value(uint32_t code_point)
	{
		return is_unicode(code_point) && !is_unicode_surrogate(code_point);
	}

	constexpr bool is_unicode_noncharacter(uint32_t code_point)
	{
		return is_unicode(code_point) && ((code_point >= 0xFDD0 && code_point <= 0xFDEF) || ((code_point & 0xFFFE) == 0xFFFE) || ((code_point & 0xFFFF) == 0xFFFF));
	}

	constexpr uint32_t to_ascii_lowercase(uint32_t code_point)
	{
		if (is_ascii_upper_alpha(code_point))
			return code_point + 0x20;
		return code_point;
	}

	constexpr uint32_t to_ascii_uppercase(uint32_t code_point)
	{
		if (is_ascii_lower_alpha(code_point))
			return code_point - 0x20;
		return code_point;
	}

	constexpr uint32_t parse_ascii_digit(uint32_t code_point)
	{
		if (is_ascii_digit(code_point))
			return code_point - '0';
		assert(false || !"Not Reached!");
	}

	constexpr uint32_t parse_ascii_hex_digit(uint32_t code_point)
	{
		if (is_ascii_digit(code_point))
			return parse_ascii_digit(code_point);
		if (code_point >= 'A' && code_point <= 'F')
			return code_point - 'A' + 10;
		if (code_point >= 'a' && code_point <= 'f')
			return code_point - 'a' + 10;
		// VERIFY_NOT_REACHED();
		assert(false || !"Not Reached!");
	}

	constexpr uint32_t parse_ascii_base36_digit(uint32_t code_point)
	{
		if (is_ascii_digit(code_point))
			return parse_ascii_digit(code_point);
		if (code_point >= 'A' && code_point <= 'Z')
			return code_point - 'A' + 10;
		if (code_point >= 'a' && code_point <= 'z')
			return code_point - 'a' + 10;
		// VERIFY_NOT_REACHED();
		assert(false || !"Not Reached!");
	}

	constexpr uint32_t to_ascii_base36_digit(uint32_t digit)
	{
		constexpr std::array<char, 36> base36_map = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
		assert(digit < base36_map.size());
		return base36_map[digit];
	}

}