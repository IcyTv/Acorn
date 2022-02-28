/*
 * Copyright (c) 2021, Tim Flynn <trflynn89@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace Acorn::IDL
{

	class Utf16View;

	class Utf16CodePointIterator
	{
		friend class Utf16View;

	public:
		Utf16CodePointIterator() = default;
		~Utf16CodePointIterator() = default;

		bool operator==(Utf16CodePointIterator const& other) const
		{
			return (m_ptr == other.m_ptr) && (m_remaining_code_units == other.m_remaining_code_units);
		}

		bool operator!=(Utf16CodePointIterator const& other) const
		{
			return !(*this == other);
		}

		Utf16CodePointIterator& operator++();
		uint32_t operator*() const;

		size_t length_in_code_units() const;

	private:
		Utf16CodePointIterator(uint16_t const* ptr, size_t length)
			: m_ptr(ptr), m_remaining_code_units(length)
		{
		}

		uint16_t const* m_ptr{nullptr};
		size_t m_remaining_code_units{0};
	};

	class Utf16View
	{
	public:
		static bool is_high_surrogate(uint16_t);
		static bool is_low_surrogate(uint16_t);
		static uint32_t decode_surrogate_pair(uint16_t high_surrogate, uint16_t low_surrogate);

		Utf16View() = default;
		~Utf16View() = default;

		explicit Utf16View(std::span<uint16_t const> code_units)
			: m_code_units(code_units)
		{
		}

		bool operator==(Utf16View const& other) const
		{
			if (m_code_units.size() != other.m_code_units.size())
				return false;

			for (size_t i = 0; i < m_code_units.size(); ++i)
				if (m_code_units[i] != other.m_code_units[i])
					return false;

			return true;
		}

		enum class AllowInvalidCodeUnits
		{
			Yes,
			No,
		};

		std::string to_utf8(AllowInvalidCodeUnits = AllowInvalidCodeUnits::No) const;

		bool is_null() const { return m_code_units.empty(); }
		bool is_empty() const { return m_code_units.empty(); }
		size_t length_in_code_units() const { return m_code_units.size(); }
		size_t length_in_code_points() const;

		Utf16CodePointIterator begin() const { return {begin_ptr(), m_code_units.size()}; }
		Utf16CodePointIterator end() const { return {end_ptr(), 0}; }

		uint16_t const* data() const { return m_code_units.data(); }
		uint16_t code_unit_at(size_t index) const;
		uint32_t code_point_at(size_t index) const;

		size_t code_point_offset_of(size_t code_unit_offset) const;
		size_t code_unit_offset_of(size_t code_point_offset) const;
		size_t code_unit_offset_of(Utf16CodePointIterator const&) const;

		Utf16View substring_view(size_t code_unit_offset, size_t code_unit_length) const;
		Utf16View substring_view(size_t code_unit_offset) const { return substring_view(code_unit_offset, length_in_code_units() - code_unit_offset); }

		Utf16View unicode_substring_view(size_t code_point_offset, size_t code_point_length) const;
		Utf16View unicode_substring_view(size_t code_point_offset) const { return unicode_substring_view(code_point_offset, length_in_code_points() - code_point_offset); }

		bool validate(size_t& valid_code_units) const;
		bool validate() const
		{
			size_t valid_code_units;
			return validate(valid_code_units);
		}

		bool equals_ignoring_case(Utf16View const&) const;

	private:
		uint16_t const* begin_ptr() const { return m_code_units.data(); }
		uint16_t const* end_ptr() const { return begin_ptr() + m_code_units.size(); }

		size_t calculate_length_in_code_points() const;

		std::span<uint16_t const> m_code_units;
		mutable std::optional<size_t> m_length_in_code_points;
	};

}