
/*
 * Copyright (c) 2022 Michael Finger
 *
 * SPDX-License-Identifier: Apache-2.0 with Commons Clause
 *
 * For more Information on the license, see the LICENSE.md file
 */

#pragma once

#include "core/Core.h"
#include "core/UUID.h"

#include <yaml-cpp/yaml.h>

namespace Acorn
{
	class AssetManager;

	enum class AssetType
	{
		Texture,
		Shader,
	};

	class Asset : std::enable_shared_from_this<Asset>
	{
public:
		virtual ~Asset() = default;

		virtual AssetType GetType() const = 0;

		inline void EnsureLoaded()
		{
			if (!m_IsLoaded)
			{
				Load();
				m_IsLoaded = true;
			}
		}

		template <typename T>
		bool Is() const
		{
			return GetType() == T::GetStaticType();
		}

		Ref<Asset> GetSharedPtr()
		{
			return shared_from_this();
		}

		template <typename T, typename... Args>
		[[nodiscard]] static Ref<Asset> Create()
		{
			static_assert(std::is_base_of<Asset, T>::value, "T must be a subclass of Asset");
			return std::make_shared<T>(Args...);
		}

		template <typename T>
		Ref<T> As()
		{
			AC_CORE_ASSERT(Is<T>());
			return std::static_pointer_cast<T>(GetSharedPtr());
		}

		// Enforce serialization of the AssetType
		virtual YAML::Emitter operator<<(YAML::Emitter& out) const = 0;

protected:
		virtual void Load() = 0;

private:
		Asset() = default;

		friend class AssetManager;

private:
		UUID m_UUID;
		bool m_IsLoaded = false;
	};

	class ShaderAsset : public Asset
	{
public:
		ShaderAsset();

		static AssetType GetStaticType()
		{
			return AssetType::Shader;
		}
		virtual AssetType GetType() const override
		{
			return GetStaticType();
		}

		virtual YAML::Emitter operator<<(YAML::Emitter& out) const override;

private:
		uint64_t m_Generation = 0;
	};

	using AssetHandle = Ref<Asset>;
} // namespace Acorn