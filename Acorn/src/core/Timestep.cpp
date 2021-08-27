#include "acpch.h"

#include "Timestep.h"

namespace Acorn
{
	float operator*(const Timestep& timestep, float value)
	{
		return timestep.m_Time * value;
	}


	float operator*(float value, const Timestep& timestep)
	{
		return timestep.m_Time * value;
	}

}

