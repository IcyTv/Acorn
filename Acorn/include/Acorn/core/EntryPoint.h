#pragma once

#include "Application.h"
#include "Core.h"
#include "Log.h"

#if defined(AC_PLATFORM_WINDOWS) || defined(AC_PLATFORM_LINUX)

extern Acorn::Application* Acorn::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Acorn::Log::Init();

	AC_CORE_INFO("Acorn Engine Initialized");

	// AC_PROFILE_BEGIN_SESSION("Startup", "AcornProfile-Startup.json");
	Acorn::Application* application = Acorn::CreateApplication({argc, argv});
	// AC_PROFILE_END_SESSION();

	application->Run();

	// AC_PROFILE_END_SESSION();

	// AC_PROFILE_BEGIN_SESSION("Shutdown", "AcornProfile-Shutdown.json");
	{
		AC_PROFILE_SCOPE("Application Shutdown");
		delete application;
	}
	// AC_PROFILE_END_SESSION();
}

	#define AC_ENTRY(class)                                                           \
		Acorn::Application* Acorn::CreateApplication(ApplicationCommandLineArgs args) \
		{                                                                             \
			return new class(args);                                                   \
		}

#else
	#error Unsupported Platform!
#endif