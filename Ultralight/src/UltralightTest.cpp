// #include <Acorn.h>
// #include <core/EntryPoint.h>

// namespace Acorn
// {

// 	class Ultralight : public Application
// 	{
// 	public:
// 		Ultralight(ApplicationCommandLineArgs args)
// 			: Application("Ultralight", args, true)
// 		{
// 			// PushLayer(new ExampleLayer());
// 		}

// 		~Ultralight()
// 		{
// 		}

// 	private:
// 	};

// 	// AC_ENTRY(Ultralight);
// 	Application* CreateApplication(ApplicationCommandLineArgs args)
// 	{
// 		return new Ultralight(args);
// 	}
// }

#include "AppCore/App.h"
#include "MyApp.h"
#include <AppCore/AppCore.h>
#include <Ultralight/Ultralight.h>
#include <cstdio>
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(void)
{
	using namespace ultralight;
	Config config;

	MyApp app;

	app.Run();

	return 0;
}