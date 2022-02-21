#include "acpch.h"

#include "core/UUID.h"

#include <boost/uuid/uuid_generators.hpp>

namespace Acorn
{
	UUID::UUID()
	{
		m_UUID = boost::uuids::random_generator()();
	}

	UUID::UUID(const std::string& uuid)
	{
		m_UUID = boost::uuids::string_generator()(uuid);
	}

}