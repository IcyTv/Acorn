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
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Utils.h"

namespace Acorn::IDL
{
	using namespace std::literals::string_view_literals; // To allow ""sv construction

	struct Type;
	struct Interface;

	template <typename FunctionType>
	static size_t GetFunctionLength(FunctionType& function)
	{
		size_t length = 0;
		for (auto& parameter : function.Parameters)
		{
			if (!parameter.Optional && !parameter.Variadic)
				length++;
		}
		return length;
	}

	enum class SequenceStorageType
	{
		Vector,		  // Used to safely store non-JS values
		MarkedVector, // Used to safely store JS::Value and anything that inherits JS::Cell, e.g. JS::Object
	};

	struct CppType
	{
		std::string Name;
		SequenceStorageType SequenceStorage;

		static CppType From(std::shared_ptr<Type> type, const Interface& interface);
	};

	struct Type
	{
		Type() = default;
		Type(const std::string& name, bool nullable) : Name(name), Nullable(nullable) {}

		virtual ~Type() = default;

		std::string Name;
		bool Nullable{ false };

		bool IsString() const
		{
			return Name == "string";
		}

		// https://webidl.spec.whatwg.org/#dfn-integer-type
		bool IsInteger() const
		{
			return IsOneOf(Name, "byte", "octet", "short", "unsigned short", "long", "unsigned long", "long long", "unsigned long long");
		}
		// https://webidl.spec.whatwg.org/#dfn-numeric-type
		bool IsNumeric() const
		{
			return IsInteger() || IsOneOf(Name, "float", "unrestricted float", "double", "unrestricted double");
		}
	};

	struct Parameter
	{
		std::shared_ptr<Type> Type;
		std::string Name;
		bool Optional{ false };
		std::optional<std::string> DefaultValue;
		std::unordered_map<std::string, std::string> Attributes;
		bool Variadic{ false };
	};

	struct Function
	{
		std::shared_ptr<Type> ReturnType;
		std::string Name;
		std::vector<Parameter> Parameters;
		std::unordered_map<std::string, std::string> ExtendedAttributes;

		size_t length() const
		{
			return GetFunctionLength(*this);
		}
	};

	struct Constructor
	{
		std::string Name;
		std::vector<Parameter> Parameters;

		size_t length() const
		{
			return GetFunctionLength(*this);
		}
	};

	struct Constant
	{
		std::shared_ptr<Type> Type;
		std::string Name;
		std::string Value;
	};

	struct Attribute
	{
		bool ReadOnly{ false };
		std::shared_ptr<Type> Type;
		std::string Name;
		std::unordered_map<std::string, std::string> ExtendedAttributes;

		std::string GetterNameCallback;
		std::string SetterNameCallback;
	};

	struct DictMember
	{
		bool Required{ false };
		std::shared_ptr<Type> Type;
		std::string Name;
		std::unordered_map<std::string, std::string> ExtendedAttributes;
		std::optional<std::string> DefaultValue;
	};

	struct Dict
	{
		std::string ParentName;
		std::vector<DictMember> Members;
	};

	struct Enum
	{
		std::unordered_set<std::string> Values;
		std::unordered_map<std::string, std::string> TranslatedCppNames;
		std::string FirstMember;
		bool IsOriginalDefinition{ true };
	};

	struct ParameterizedType : public Type
	{
		ParameterizedType() = default;
		ParameterizedType(const std::string& name, bool nullable, const std::vector<std::shared_ptr<Type>>& parameters) : Type(name, nullable), Parameters(parameters) {}

		virtual ~ParameterizedType() = default;

		std::vector<std::shared_ptr<Type>> Parameters;

		// void GenerateSequenceFromIterable()
	};

	struct Interface
	{
		std::string Name;
		std::string ParentName;

		bool IsMixin{ false };

		std::unordered_map<std::string, std::string> ExtendedAttributes;

		std::vector<Attribute> Attributes;
		std::vector<Constant> Constants;
		std::vector<Constructor> Constructors;
		std::vector<Function> Functions;
		std::vector<Function> StaticFunctions;
		bool HasStringifier{ false };
		std::optional<std::string> StringifierAttribute;
		bool HasUnscopableMember{ false };

		std::optional<std::shared_ptr<Type>> ValueIteratorType;
		std::optional<std::pair<std::shared_ptr<Type>, std::shared_ptr<Type>>> PairIteratorTypes;

		std::optional<Function> NamedPropertyGetter;
		std::optional<Function> NamedPropertySetter;
		std::optional<Function> NamedPropertyDeleter;

		std::optional<Function> IndexedPropertyGetter;
		std::optional<Function> IndexedPropertySetter;

		std::unordered_map<std::string, Dict> Dictionaries;
		std::unordered_map<std::string, Enum> Enums;
		std::unordered_map<std::string, std::shared_ptr<Interface>> Mixins;

		// Added for convinience after parsing
		std::string WrapperClass;
		std::string WrapperBaseClass;
		std::string FullyQualifiedName;
		std::string ConstructorClass;
		std::string PrototypeClass;
		std::string PrototypeBaseClass;
		std::unordered_map<std::string, std::unordered_set<std::string>> IncludedMixins;

		std::string ModuleOwnPath;
		std::unordered_set<std::string> ImportedPaths;
		std::vector<std::shared_ptr<Interface>> ImportedModules;

		// https://webidl.spec.whatwg.org/#dfn-support-indexed-properties
		bool SupportsIndexedProperties() const
		{
			return IndexedPropertyGetter.has_value();
		}

		// https://webidl.spec.whatwg.org/#dfn-support-named-properties
		bool SupportsNamedProperties() const
		{
			return NamedPropertyGetter.has_value();
		}

		// https://webidl.spec.whatwg.org/#dfn-legacy-platform-object
		bool IsLegacyPlatformObject() const
		{
			return !ExtendedAttributes.contains("Global") && (SupportsIndexedProperties() || SupportsNamedProperties());
		}
	};

	struct AnnotatedType : public Type
	{
		AnnotatedType() = default;
		AnnotatedType(const std::string& name, bool nullable, std::unordered_map<std::string, std::string> extendedAttributes)
		: Type(name, nullable), ExtendedAttributes(extendedAttributes)
		{
		}

		virtual ~AnnotatedType() = default;

		std::unordered_map<std::string, std::string> ExtendedAttributes;
	};

	struct UnionType : public Type
	{
		UnionType() = default;
		UnionType(const std::string& name, bool nullable, const std::vector<std::shared_ptr<Type>>& parameters) : Type(name, nullable), MemberTypes(parameters) {}

		virtual ~UnionType() = default;

		std::vector<std::shared_ptr<Type>> MemberTypes;

		// https://webidl.spec.whatwg.org/#dfn-flattened-union-member-types
		std::vector<std::shared_ptr<Type>> FlattenedMemberTypes() const
		{
			// 1. Let T be the union type (✔️)

			// 2. Initialize S to {}.
			std::vector<std::shared_ptr<Type>> types;

			// 3. For each member type U of T:
			for (auto type : MemberTypes)
			{
				// 1. If U is an annotaded type, the set u to be the inner type of U.
				if (Is<AnnotatedType>(type))
				{
					// FIXME: do we even need to do anything here? We already have the inner type, since annotated types are wrappers around type...
				}

				// 2. If U is a nullable type, then set U to be the inner type of U. (NOTE: Not necessary as nullable is stored with Type and not as a separate struct)

				// 3. If U is a union type, then add to S the flattened member types of U.
				if (Is<UnionType>(type))
				{
					auto unionMemberType = VerifyCast<UnionType>(type);
					auto unionTypes		 = unionMemberType->FlattenedMemberTypes();
					types.reserve(types.size() + unionTypes.size());
					types.insert(types.end(), unionTypes.begin(), unionTypes.end());
				}
				else
				{
					// 4. Otherwise U is not a union type. Add U to S.
					types.push_back(type);
				}
			}

			// 4. Return S.
			return types;
		}

		// https://webidl.spec.whatwg.org/#dfn-number-of-nullable-member-types
		size_t NumberOfNullableMemberTypes() const
		{
			// 1. Let T be the union type (✔️)

			// 2. Initialize N to 0.
			size_t numberOfNullableTypes = 0;

			// 3. For each member type U of T:
			for (auto type : MemberTypes)
			{
				// 1. If U is a nullable type, then increment N by 1.
				if (type->Nullable)
				{
					// 1. Set n to n + 1.
					numberOfNullableTypes++;

					// 2. Set U to be the inner type of U. (NOTE: Not necessary as nullable is stored with Type and not as a separate struct)
				}

				// 2. If U is a union type, then:
				if (Is<UnionType>(type))
				{
					auto unionMemberType = VerifyCast<UnionType>(type);

					// 1. Let m be the number of nullable member types of U.
					// 2. Set n to n + m.
					numberOfNullableTypes += unionMemberType->NumberOfNullableMemberTypes();
				}
			}

			// 4. Return N.
			return numberOfNullableTypes;
		}

		// https://webidl.spec.whatwg.org/#dfn-includes-a-nullable-type
		bool IncludesNullableType() const
		{
			return std::find_if(MemberTypes.begin(), MemberTypes.end(), [](const std::shared_ptr<Type>& type) { return type->Nullable; }) != MemberTypes.end();
		}

		// https://webidl.spec.whatwg.org/#dfn-includes-undefined
		bool IncludesUndefined() const
		{
			// The type is a union type and one of its member types includes undefined.
			for (auto type : MemberTypes)
			{
				if (Is<UnionType>(type))
				{
					auto unionMemberType = VerifyCast<UnionType>(type);
					if (unionMemberType->IncludesUndefined())
					{
						return true;
					}
				}

				if (type->Name == "undefined"sv)
				{
					return true;
				}
			}

			return false;
		}

		std::string ToVariant(const Interface& interface) const
		{
			std::stringstream builder;
			builder << "Variant<";

			auto flattenedTypes = FlattenedMemberTypes();
			for (size_t typeIdx = 0; typeIdx < flattenedTypes.size(); typeIdx++)
			{
				auto& type = flattenedTypes.at(typeIdx);

				if (typeIdx > 0)
				{
					builder << ", ";
				}

				auto cppType = CppType::From(type, interface);
				builder << cppType.Name;
			}

			if (IncludesUndefined())
			{
				builder << ", Empty";
			}

			builder << ">";
			return builder.str();
		}
	};
} // namespace Acorn::IDL