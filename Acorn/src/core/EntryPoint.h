#pragma once

#ifdef AC_PLATFORM_WINDOWS

extern Acorn::Application* Acorn::CreateApplication();

int main(int argc, char** argv)
{
	Acorn::Log::Init();

	AC_PROFILE_BEGIN_SESSION("Startup", "AcornProfile-Startup.json");
	Acorn::Application* application = Acorn::CreateApplication();
	AC_PROFILE_END_SESSION();

	application->Run();

	AC_PROFILE_END_SESSION();

	AC_PROFILE_BEGIN_SESSION("Shutdown", "AcornProfile-Shutdown.json");
	delete application;
	AC_PROFILE_END_SESSION();
}


#define AC_ENTRY(class)  Acorn::Application* Acorn::CreateApplication() { \
	return new class;\
}

#else
#error Only Windows for now!
#endif