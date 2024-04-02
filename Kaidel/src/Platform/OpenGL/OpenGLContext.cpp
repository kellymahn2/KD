#include "KDpch.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Kaidel {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KD_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		
		int status = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
		KD_CORE_ASSERT(status, "Failed to initialize Glad!");
		KD_CORE_INFO("OpenGL Info:");
		KD_CORE_INFO("  Vendor: {0}", std::string((const char*)glGetString(GL_VENDOR)));
		KD_CORE_INFO("  Renderer: {0}", std::string((const char*)glGetString(GL_RENDERER)));
		KD_CORE_INFO("  Version: {0}", std::string((const char*)glGetString(GL_VERSION)));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}
