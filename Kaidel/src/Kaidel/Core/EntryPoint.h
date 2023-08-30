#pragma once
#include "Kaidel/Core/Base.h"

#ifdef KD_PLATFORM_WINDOWS

extern Application* CreateApplication();

int main(int argc, char** argv)
{
	Log::Init();

	KD_PROFILE_BEGIN_SESSION("Startup", "KaidelProfile-Startup.json");
	auto app = CreateApplication();
	KD_PROFILE_END_SESSION();

	KD_PROFILE_BEGIN_SESSION("Runtime", "KaidelProfile-Runtime.json");
	app->Run();
	KD_PROFILE_END_SESSION();

	KD_PROFILE_BEGIN_SESSION("Shutdown", "KaidelProfile-Shutdown.json");
	delete app;
	KD_PROFILE_END_SESSION();
}

#endif
