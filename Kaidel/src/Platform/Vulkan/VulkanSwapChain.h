#pragma once
#include <glad/vulkan.h>


#include "VulkanLogicalDevice.h"
#include "VulkanPhysicalDevice.h"


struct GLFWwindow;

namespace Kaidel {


	struct SwapChainSpecification {
		VkFormat SwapChainFormat;
		VkPresentModeKHR PresentMode; 
		VkColorSpaceKHR ColorSpace;
		uint32_t SwapChainWidth;
		uint32_t SwapChainHeight;
		uint32_t SwapChainImageCount;
		GLFWwindow* Window;
	};


	class VulkanSwapChain {
	public:
		const SwapChainSpecification& GetSpecification()const { return m_Specification; }

		VkSwapchainKHR GetSwapChain()const { return m_SwapChain; }


		static void Init(const SwapChainSpecification& specification);
		static void Shutdown();

	private:
		void Invalidate(const SwapChainSpecification& spec);
		void Cleanup();
	private:
		SwapChainSpecification m_Specification;
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

		std::vector<VkImage> m_SwapChainImages;


		static Scope<VulkanSwapChain> s_VulkanSwapChain;
	};

}
