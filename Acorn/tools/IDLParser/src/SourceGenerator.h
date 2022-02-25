/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "GenericLexer.h"

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Acorn::IDL
{
	class SourceGenerator
	{
	public:
		using MappingType = std::unordered_map<std::string_view, std::string>;

		explicit SourceGenerator(std::stringstream& builder, char opening = '@', char closing = '@')
			: m_Builder(builder), m_Opening(opening), m_Closing(closing)
		{
		}

		explicit SourceGenerator(std::stringstream& builder, const MappingType& mapping, char opening = '@', char closing = '@')
			: m_Builder(builder), m_Mapping(mapping), m_Opening(opening), m_Closing(closing)
		{
		}

		SourceGenerator Fork() { return SourceGenerator{m_Builder, m_Mapping, m_Opening, m_Closing}; }

		inline void Set(std::string_view Key, std::string Value) { m_Mapping[Key] = Value; }
		inline std::string Get(std::string_view Key) const { return m_Mapping.at(Key); }

		std::string_view AsStringView() const { return m_Builder.str(); }
		std::string AsString() const { return m_Builder.str(); }

		void Append(std::string_view pattern)
		{
			GenericLexer lexer{pattern};

			while (!lexer.is_eof())
			{
				const auto consumeWithoutConsumingStopChar = [&](char stop)
				{
					return lexer.consume_while([&](char c)
											   { return c != stop; });
				};

				m_Builder << consumeWithoutConsumingStopChar(m_Opening);

				if (lexer.consume_specific(m_Opening))
				{
					const auto placeholder = consumeWithoutConsumingStopChar(m_Closing);

					if (!lexer.consume_specific(m_Closing))
						throw std::runtime_error("Expected closing character");

					m_Builder << Get(placeholder);
				}
				else
				{
					assert(lexer.is_eof());
				}
			}
		}

	private:
		SourceGenerator(const SourceGenerator&) = delete;
		SourceGenerator& operator=(const SourceGenerator&) = delete;

	private:
		std::stringstream& m_Builder;
		char m_Opening;
		char m_Closing;
		MappingType m_Mapping;
	};

}