/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2021, Luke Wilde <lukew@serenityos.org>
 * Copyright (c) 2022, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "GenericLexer.h"
#include "IDLTypes.h"

#include <memory>
#include <optional>
#include <string_view>

namespace Acorn::IDL
{
	class Parser
	{
	public:
		Parser(std::string_view filename, std::string_view contents, std::string_view importBasePath);
		virtual ~Parser() {}

		std::shared_ptr<Interface> Parse();

	private:
		void AssertSpecific(char ch);
		void AssertString(std::string_view expected);
		void ConsumeWhitespace();

		std::optional<std::shared_ptr<Interface>> ResolveImport(auto path);

		std::unordered_map<std::string, std::string> ParseExtendedAttributes();
		void ParseAttribute(std::unordered_map<std::string, std::string>& extendedAttributes, Interface& interface);
		void ParseInterface(Interface&);
		void ParseNonInterfaceEntities(bool allowInterface, Interface&);
		void ParseEnum(Interface&);
		void ParseInterfaceMixin(Interface&);
		void ParseDictionary(Interface&);
		void ParseConstructor(Interface&);
		void ParseGetter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface&);
		void ParseSetter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface&);
		void ParseDeleter(std::unordered_map<std::string, std::string>& extendedAttributes, Interface&);
		void ParseStringifier(std::unordered_map<std::string, std::string>& extendedAttributes, Interface&);
		void ParseIterable(Interface&);
		void ParseConstant(Interface&);
		Function ParseFunction(std::unordered_map<std::string, std::string>& extendedAttributes, Interface&, bool isSpecialOperation = false);
		std::vector<Parameter> ParseParameters();
		std::shared_ptr<Type> ParseType();

		static std::unordered_set<std::string> s_AllImportedPaths;
		std::string m_ImportBasePath;
		std::string m_Filename;
		std::string m_Input;
		GenericLexer m_Lexer;
	};
}