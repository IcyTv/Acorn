#pragma once

#include "Application.h"
#include "Core.h"
#include "Log.h"

#if (defined(AC_PLATFORM_WINDOWS) || defined(AC_PLATFORM_LINUX)) && !defined(AC_TEST)

extern Acorn::Application* Acorn::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Acorn::Log::Init();

	AC_CORE_INFO("Acorn Engine Initialized");

	Acorn::Application* application = Acorn::CreateApplication({argc, argv});

	application->Run();

	{
		AC_PROFILE_SCOPE("Application Shutdown");
		delete application;
	}
}

#else
	#error Unsupported Platform!
#endif

#define AC_ENTRY(className)                                             \
	namespace Acorn                                                     \
	{                                                                   \
		Application* CreateApplication(ApplicationCommandLineArgs args) \
		{                                                               \
			return new className(args);                                 \
		}                                                               \
	}