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

#include <cassert>
#include <memory>
#include <string>

namespace Acorn::IDL
{

	template <class T>
	using AddConst = const T;

	template <class T>
	struct __RemoveConst
	{
		using Type = T;
	};
	template <class T>
	struct __RemoveConst<const T>
	{
		using Type = T;
	};
	template <class T>
	using RemoveConst = typename __RemoveConst<T>::Type;

	template <class T>
	inline constexpr bool IsConst = false;

	template <class T>
	inline constexpr bool IsConst<const T> = true;

	template <bool condition, class TrueType, class FalseType>
	struct __Conditional
	{
		using Type = TrueType;
	};

	template <class TrueType, class FalseType>
	struct __Conditional<false, TrueType, FalseType>
	{
		using Type = FalseType;
	};

	template <bool condition, class TrueType, class FalseType>
	using Conditional = typename __Conditional<condition, TrueType, FalseType>::Type;

	template <typename ReferenceType, typename T>
	using CopyConst = Conditional<IsConst<ReferenceType>, AddConst<T>, RemoveConst<T>>;

	template <typename OutputType, typename InputType>
	inline bool Is(std::shared_ptr<InputType>& input)
	{
		return dynamic_cast<OutputType*>(input.get()) != nullptr;
	}

	template <typename OutputType, typename InputType>
	inline bool Is(InputType& input)
	{
		// if constexpr (requires { input.template fast_is<OutputType>(); })
		// {
		// 	return input.template fast_is<OutputType>();
		// }
		return dynamic_cast<CopyConst<InputType, OutputType>*>(&input);
	}

	template <typename OutputType, typename InputType>
	inline bool Is(InputType* input)
	{
		return input && Is<OutputType>(*input);
	}

	template <typename OutputType, typename InputType>
	inline CopyConst<InputType, OutputType>* VerifyCast(InputType* input)
	{
		// static_assert(IsBaseOf<InputType, OutputType>);
		assert(!input || Is<OutputType>(*input));
		return static_cast<CopyConst<InputType, OutputType>*>(input);
	}

	template <typename OutputType, typename InputType>
	inline CopyConst<InputType, OutputType>& VerifyCast(InputType& input)
	{
		// static_assert(IsBaseOf<InputType, OutputType>);
		assert(Is<OutputType>(input));
		return static_cast<CopyConst<InputType, OutputType>&>(input);
	}

	template <typename OutputType, typename InputType>
	inline std::shared_ptr<OutputType> VerifyCast(std::shared_ptr<InputType> input)
	{
		// static_assert(IsBaseOf<InputType, OutputType>);
		assert(Is<OutputType>(input));
		return std::static_pointer_cast<OutputType>(input);
	}

	constexpr inline bool IsEqual(const char* one, const char* two)
	{
		while (*one != '\0' && *two != '\0')
			if (*one++ != *two++)
				return false;

		return true;
	}

	// FIXME there is certainly a better way to do this
	template <typename... Ts>
	constexpr inline bool IsOneOf(const std::string& str, Ts&&... ts)
	{
		return (... || IsEqual(str.c_str(), std::forward<Ts>(ts)));
	}
}