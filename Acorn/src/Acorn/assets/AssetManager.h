
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "core/Core.h"

#include "assets/Asset.h"

#include <filesystem>

namespace Acorn
{
	class AssetException : public std::runtime_error
	{
public:
		AssetException(const std::string& message) : std::runtime_error(message) {}
	};

	class AssetManager
	{
public:
		static AssetManager Instance()
		{
			static AssetManager instance;
			return instance;
		}

		virtual ~AssetManager();

		template <AssetType Type>
		AssetHandle Get(std::string_view assetName);

		template <AssetType Type>
		AssetHandle Get(const std::filesystem::path& assetPath);

		template <AssetType Type>
		AssetHandle Get(const UUID& uuid);

		void BulkLoad(const std::filesystem::path& path);

		/**
		 * @brief Makes sure no assets are held.
		 *
		 * This allows for all the assets to be freed and saved to disk.
		 */
		void Flush();

private:
		AssetManager();

		AssetHandle LoadMeta(const std::filesystem::path& path);
		AssetType ResolveAssetType(const std::filesystem::path& path);

private:
		std::unordered_map<UUID, AssetHandle> m_Assets;
		std::unordered_map<std::string, UUID> m_AssetNames;
		std::vector<std::filesystem::path> m_BasePaths;
	};

} // namespace Acorn