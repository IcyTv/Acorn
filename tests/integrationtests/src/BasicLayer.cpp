#include "BasicLayer.h"
namespace Tests
{
	BasicLayer::BasicLayer()
		: Layer("BasicLayer")
	{
		m_BasicEntity = m_Scene->CreateEntity("BasicEntity");
	}
}