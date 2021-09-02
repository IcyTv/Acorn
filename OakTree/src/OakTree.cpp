#include <Acorn.h>
#include <core/EntryPoint.h>

#include "OakLayer.h"

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

	AC_ENTRY(OakTree)

}
