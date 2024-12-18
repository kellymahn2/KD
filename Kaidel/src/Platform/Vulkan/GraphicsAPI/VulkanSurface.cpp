#include "KDpch.h"
#include "VulkanSurface.h"
#include "VulkanGraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Kaidel {
	VulkanSurface::VulkanSurface(void* window)
		:m_Surface(VK_NULL_HANDLE)
	{
		VK_ASSERT(glfwCreateWindowSurface(VK_INSTANCE.GetInstance(), (GLFWwindow*)window, nullptr, &m_Surface));
	}
	VulkanSurface::~VulkanSurface()
	{
		vkDestroySurfaceKHR(VK_INSTANCE.GetInstance(), m_Surface, nullptr);
	}
}
