#pragma once
#include "Kaidel/Core/Base.h"

#ifdef KD_PLATFORM_WINDOWS


extern Kaidel::Application* Kaidel::CreateApplication(const Kaidel::ApplicationCommandLineArgs& specification);

int main(int argc, char** argv)
{
	Kaidel::Log::Init();

	auto app = Kaidel::CreateApplication({argc,argv});

	app->Run();

	delete app;
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
