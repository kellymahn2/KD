#pragma once
#include "Kaidel/Core/Base.h"

#ifdef KD_PLATFORM_WINDOWS


extern Kaidel::Application* Kaidel::CreateApplication(const Kaidel::ApplicationCommandLineArgs& specification);

int main(int argc, char** argv)
{
	Log::Init();

	KD_PROFILE_BEGIN_SESSION("Startup", "KaidelProfile-Startup.json");
	auto app = Kaidel::CreateApplication({argc,argv});
	KD_PROFILE_END_SESSION();

	KD_PROFILE_BEGIN_SESSION("Runtime", "KaidelProfile-Runtime.json");
	app->Run();
	KD_PROFILE_END_SESSION();

	KD_PROFILE_BEGIN_SESSION("Shutdown", "KaidelProfile-Shutdown.json");
	delete app;
	KD_PROFILE_END_SESSION();
}
#ifdef KAIDEL_DIST
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
) {
	main(0, nullptr);
}
#endif	
#endif
