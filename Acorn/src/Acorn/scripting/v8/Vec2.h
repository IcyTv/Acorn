/**
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */
 #pragma once

namespace Acorn::Scripting::V8 {
	class Vec2 {
	public:
		explicit Vec2(double x, double  y) : x(x), y(y) {}
		double  x;
		double  y;

		double  Length() const {
			return sqrt(x * x + y * y);
		}

		double  LengthSquared() const {
			return x * x + y * y;
		}

		double IndexedGet(uint32_t index) const {
			if (index == 0) {
				return x;
			}
			if (index == 1) {
				return y;
			}
			throw std::out_of_range("index out of range");
		}

		double IndexedSet(uint32_t index, double value) {
			if (index == 0) {
				x = value;
				return x;
			}
			if (index == 1) {
				y = value;
				return y;
			}
			throw std::out_of_range("index out of range");
		}
	};
}