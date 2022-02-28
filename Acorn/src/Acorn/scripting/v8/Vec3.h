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

		double IndexedGet(uint32_t index) const {
			if(index == 0) {
				return x;
			}
			if(index == 1) {
				return y;
			}
			if(index == 2) {
				return z;
			}
			throw std::out_of_range("Index out of range");
		}

		void IndexedSet(uint32_t index, double value) {
			if(index == 0) {
				x = value;
			}
			if(index == 1) {
				y = value;
			}
			if(index == 2) {
				z = value;
			}
			throw std::out_of_range("Index out of range");
		}
	};
}