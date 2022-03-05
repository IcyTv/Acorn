
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#include "acpch.h"

#include "assets/AssetManager.h"

#include <boost/algorithm/string.hpp>
#include <magic_enum.hpp>

#include <fstream>

namespace Acorn
{
	// TODO we only should allow paths to be used in non-release builds. I.e. in OakTree.
	// TODO instead of asserting, we should throw exceptions, so we have a chance to recover.

	AssetManager::AssetManager()
	{
		m_BasePaths.reserve(2);
		m_BasePaths.emplace_back(std::filesystem::current_path() / "assets");
		// m_BasePaths.emplace_back(""); //TODO we should get a global assets path?
	}

	AssetManager::~AssetManager()
	{
		Flush();
	}

	template <AssetType AssetType>
	AssetHandle AssetManager::Get(std::string_view assetName)
	{
		return Get<AssetType>(m_AssetNames[assetName]);
	}

	template <AssetType AssetType>
	AssetHandle AssetManager::Get(const std::filesystem::path& assetPath)
	{
	}

	template <AssetType Type>
	AssetHandle AssetManager::Get(const UUID& uuid)
	{
		static_assert(false, "AssetType not implemented for this Type");
	}

	void AssetManager::BulkLoad(const std::filesystem::path& path)
	{
		AC_ASSERT_NOT_REACHED();
	}

	void AssetManager::Flush()
	{
		for (auto& [_, asset] : m_Assets)
		{
			AC_CORE_ASSERT(asset.use_count() == 1, "Asset is still in use");
		}

		m_Assets.clear();
	}

	AssetHandle AssetManager::LoadMeta(const std::filesystem::path& path)
	{
		AC_CORE_ASSERT(std::filesystem::exists(path), "Asset does not exist");

		std::filesystem::path metaPath = path;
		metaPath.replace_extension(".meta");

		if (!std::filesystem::exists(metaPath))
		{
			// Generate a meta file.
			YAML::Emitter out;
			if (std::filesystem::is_directory(metaPath))
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Type" << YAML::Value << "Folder";
				out << YAML::Key << "Uuid" << YAML::Value << UUID{};
				out << YAML::EndMap;
			}
			else
			{
				AssetType assetType = ResolveAssetType(path);
				out << YAML::BeginMap;
				out << YAML::Key << "Type" << YAML::Value << magic_enum::enum_name(assetType).data();
				out << YAML::Key << "Uuid" << YAML::Value << UUID{};
				out << YAML::EndMap;
			}

			std::ofstream metaFile(metaPath);
			metaFile << out.c_str();
			metaFile.close();
		}
		else
		{
			// Load the meta file.
		}

		return {};
	}

	AssetType AssetManager::ResolveAssetType(const std::filesystem::path& path)
	{
		static const char* const TEXTURE_EXTENSIONS[] = { ".png", ".jpg", ".jpeg", ".bmp" };
		std::string extension						  = path.extension().string();
		boost::algorithm::to_lower(extension);

		if (std::find(std::begin(TEXTURE_EXTENSIONS), std::end(TEXTURE_EXTENSIONS), extension) != std::end(TEXTURE_EXTENSIONS))
		{
			return AssetType::Texture;
		}

		AC_CORE_ERROR("Unknown asset type for file: {0}", path.string());
		AC_ASSERT_NOT_REACHED();
		return AssetType::Texture;
	}
} // namespace Acorn