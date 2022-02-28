/**
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */
 #pragma once

namespace Acorn::Scripting::V8 {
	class Vec3 {
	public:
		explicit Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
		double x;
		double y;
		double z;

		double  Length() const {
			return sqrt(x * x + y * y + z * z);
		}

		double  LengthSquared() const {
			return x * x + y * y + z * z;
		}
	};
}