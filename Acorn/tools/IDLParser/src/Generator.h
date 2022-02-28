/**
 * Copyright (c) 2020, Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * Read the LICENSE.md file for details.
 */
#pragma once

#include "IDLTypes.h"

#include <string>
#include <filesystem>

namespace Acorn::IDL
{
	void GenerateHeader(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath = "./");
	void GenerateImplementation(const Interface& interface, std::filesystem::path outPath, std::string templateBasePath = "./");
}