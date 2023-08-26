#include "KDpch.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <Glad/include/glad/glad.h>

namespace Kaidel {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KD_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		KD_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KD_CORE_ASSERT(status, "Failed to initialize Glad!");
		KD_CORE_INFO("OpenGL Info:");
		KD_CORE_INFO("  Vendor: {0}", std::string((const char*)glGetString(GL_VENDOR)));
		KD_CORE_INFO("  Renderer: {0}", std::string((const char*)glGetString(GL_RENDERER)));
		KD_CORE_INFO("  Version: {0}", std::string((const char*)glGetString(GL_VERSION)));

		KD_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Kaidel requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		KD_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}
