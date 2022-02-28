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
	};
}