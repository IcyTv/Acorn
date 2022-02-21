#include "layer/Layer.h"
#include "acpch.h"

namespace Acorn
{
	Layer::Layer(const std::string& debugName)
		: m_DebugName(debugName)
	{
	}

	Layer::~Layer()
	{
	}
}