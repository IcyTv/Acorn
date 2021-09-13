#include <Acorn.h>
#include <core/EntryPoint.h>

#include "OakLayer.h"

#include <iostream>

namespace Acorn
{

	class OakTree : public Application
	{
	public:
		OakTree(ApplicationCommandLineArgs args)
			: Application("OakTree", args, true)
		{
			//PushLayer(new ExampleLayer());
			PushLayer(new OakLayer());
		}

		~OakTree()
		{
		}
	};

	// AC_ENTRY(OakTree);
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new OakTree(args);
	}
}