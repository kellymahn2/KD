#include "KDpch.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"


namespace Kaidel {

	Scope<VulkanSurface> VulkanSurface::s_VulkanSurface;


	void VulkanSurface::Init(GLFWwindow* context, VkInstance instance)
	{
		s_VulkanSurface = CreateScope<VulkanSurface>();
		s_VulkanSurface->m_Instance = instance;
		s_VulkanSurface->m_Context = context;
		KD_CORE_ASSERT(glfwCreateWindowSurface(s_VulkanSurface->m_Instance, s_VulkanSurface->m_Context, nullptr, &s_VulkanSurface->m_Surface) == VK_SUCCESS);
	}

	void VulkanSurface::Shutdown()
	{
		vkDestroySurfaceKHR(s_VulkanSurface->m_Instance, s_VulkanSurface->m_Surface, nullptr);
	}

	VulkanSurface::~VulkanSurface()
	{
	}

}


