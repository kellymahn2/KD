#include "KDpch.h"
#include "VulkanSwapChain.h"
#include "VulkanSurface.h"
#include "VulkanContext.h"



#include <GLFW/glfw3.h>
namespace Kaidel {



	static uint32_t DeviceSwapChainSupportsFormat(const std::vector<VkSurfaceFormatKHR>& formatsSupportedByDevice,VkFormat format,VkColorSpaceKHR colorSpace) {
		uint32_t i = 0;
		for (auto& surfaceFormat : formatsSupportedByDevice) {
			if (surfaceFormat.format == format && surfaceFormat.colorSpace == colorSpace)
				return i;
			++i;
		}
		return -1;
	}

	static uint32_t DeviceSwapChainSupportsPresentMode(const std::vector<VkPresentModeKHR>& presentModesSupportedByDevice, VkPresentModeKHR presentMode) {
		uint32_t i = 0;
		for (auto& supportedPresentMode : presentModesSupportedByDevice) {
			if (supportedPresentMode == presentMode)
				return i;
			++i;
		}
		return -1;
	}


	Scope<VulkanSwapChain> VulkanSwapChain::s_VulkanSwapChain;


	void VulkanSwapChain::Init(const SwapChainSpecification& specification)
	{
		if(!s_VulkanSwapChain)
			s_VulkanSwapChain = CreateScope<VulkanSwapChain>();
		s_VulkanSwapChain->Invalidate(specification);
	}

	void VulkanSwapChain::Shutdown()
	{
		s_VulkanSwapChain->Cleanup();
	}
	
	void VulkanSwapChain::Cleanup() {

		if (m_SwapChain)
			vkDestroySwapchainKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, nullptr);
	}

	void VulkanSwapChain::Invalidate(const SwapChainSpecification& spec) {

		Cleanup();

		const SwapChainSupportDetails& deviceSwapChainSupportDetails = VulkanPhysicalDevice::Get().GetSwapChainSupportDetails();
		KD_CORE_ASSERT(DeviceSwapChainSupportsFormat(deviceSwapChainSupportDetails.Formats, spec.SwapChainFormat, spec.ColorSpace) != -1);
		KD_CORE_ASSERT(DeviceSwapChainSupportsPresentMode(deviceSwapChainSupportDetails.PresentModes, spec.PresentMode) != -1);

		VkExtent2D swapChainSize;

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(spec.SwapChainWidth),
			static_cast<uint32_t>(spec.SwapChainHeight)
		};

		actualExtent.width = std::clamp(actualExtent.width, deviceSwapChainSupportDetails.Capabilities.minImageExtent.width, deviceSwapChainSupportDetails.Capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, deviceSwapChainSupportDetails.Capabilities.minImageExtent.height, deviceSwapChainSupportDetails.Capabilities.maxImageExtent.height);
		swapChainSize = actualExtent;


		uint32_t imageCount = std::max(deviceSwapChainSupportDetails.Capabilities.minImageCount +1, spec.SwapChainImageCount);
		imageCount = std::min(imageCount, deviceSwapChainSupportDetails.Capabilities.maxImageCount);


		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = VulkanSurface::Get().GetSurface();


		swapChainCreateInfo.minImageCount = imageCount;
		swapChainCreateInfo.imageFormat = spec.SwapChainFormat;
		swapChainCreateInfo.imageColorSpace = spec.ColorSpace;
		swapChainCreateInfo.imageExtent = actualExtent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


		auto& graphicsQueue = VulkanLogicalDevice::Get().GetVulkanQueue(VULKAN_GRAPHICS_QUEUE_NAME);
		auto& presentQueue = VulkanLogicalDevice::Get().GetVulkanQueue(VULKAN_PRESENT_QUEUE_NAME);

		uint32_t queueFamilyIndices[] = {graphicsQueue.QueueFamilyIndex, presentQueue.QueueFamilyIndex};

		if ( presentQueue.QueueFamilyIndex != graphicsQueue.QueueFamilyIndex) {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainCreateInfo.queueFamilyIndexCount = 2;
			swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
			swapChainCreateInfo.pQueueFamilyIndices = nullptr;
		}

		swapChainCreateInfo.preTransform = deviceSwapChainSupportDetails.Capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.clipped = VK_TRUE;

		KD_CORE_ASSERT(vkCreateSwapchainKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), &swapChainCreateInfo, nullptr, &m_SwapChain) == VK_SUCCESS);


		vkGetSwapchainImagesKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(VulkanLogicalDevice::Get().GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_Specification.SwapChainImageCount = imageCount;
		m_Specification.ColorSpace = spec.ColorSpace;
		m_Specification.PresentMode = spec.PresentMode;
		m_Specification.SwapChainFormat = spec.SwapChainFormat;
		m_Specification.Window = spec.Window;
		m_Specification.SwapChainWidth = actualExtent.width;
		m_Specification.SwapChainHeight = actualExtent.height;
	}

}
