#pragma once

#include <Acorn.h>

namespace Tests
{
	using namespace Acorn;

	class BasicLayer : public Layer
	{
	public:
		BasicLayer();

	private:
		Ref<Scene> m_Scene;

		Entity m_BasicEntity;
	};
}