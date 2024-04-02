#pragma once
#include "VulkanPhysicalDevice.h"

#include <glad/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Kaidel {


	class VulkanSurface {
	public:
		VkSurfaceKHR GetSurface()const { return m_Surface; }
		
		static void Init(GLFWwindow* context, VkInstance instance);
		static void Shutdown();
		
		static VulkanSurface& Get() { return *s_VulkanSurface; }


		~VulkanSurface();
	private:
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkInstance m_Instance = VK_NULL_HANDLE;
		GLFWwindow* m_Context = nullptr;

		static Scope<VulkanSurface> s_VulkanSurface;
	};

}
