#pragma once

#include "core/Core.h"

#include <boost/container_hash/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace Acorn
{
	class UUID
	{
	public:
		UUID();
		UUID(const std::string& uuid);
		UUID(const UUID& uuid) = default;

		inline const boost::uuids::uuid& getUUID() const { return m_UUID; }

		inline operator std::string() { return to_string(m_UUID); }
		inline operator const std::string() const { return to_string(m_UUID); }

	private:
		//NOTE does this need to be a boost::uuid::uuid or is a uint64_t more performant?
		boost::uuids::uuid m_UUID;
	};
}

namespace std
{
	template <>
	struct hash<Acorn::UUID>
	{
		boost::hash<boost::uuids::uuid> uuid_hasher;

		size_t operator()(const Acorn::UUID& uuid) const
		{
			return uuid_hasher(uuid.getUUID());
		}
	};
}