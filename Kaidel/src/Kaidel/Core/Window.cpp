#include "KDpch.h"
#include "Kaidel/Core/Window.h"

#ifdef KD_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Kaidel
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
	#ifdef KD_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
	#else
		KD_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
	#endif
	}

}