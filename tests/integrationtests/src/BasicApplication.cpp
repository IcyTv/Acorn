#include <Acorn.h>
#include <gtest/gtest.h>

class BasicApp : public Acorn::Application
{
public:
	BasicApp(Acorn::ApplicationCommandLineArgs args)
		: Application("BasicApp", args, true)
	{
	}

	~BasicApp()
	{
	}
};

namespace Acorn
{
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new BasicApp(args);
	}
}