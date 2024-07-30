#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"


namespace Kaidel {
	struct SwapchainFrame {
		VkFramebuffer Framebuffer;
		VkImage Image;
		VkImageView ImageView;
	};
	
	class VulkanSwapchain {
	public:
		VulkanSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR wantedSurfaceFormat,
			VkPresentModeKHR wantedPresentMode, uint32_t wantedWidth, uint32_t wantedHeight, uint32_t wantedImageCount,
			const std::vector<uint32_t>& queueFamilyIndices);
		~VulkanSwapchain();
	
		VkSwapchainKHR GetSwapchain()const { return m_Swapchain; }
		std::vector<SwapchainFrame>& GetFrames() { return m_Frames; }
		VkSurfaceFormatKHR GetFormat()const { return m_Format; }
		VkPresentModeKHR GetPresentMode()const { return m_PresentMode; }
		VkExtent2D GetExtent()const { return m_Extent; }
		uint32_t GetImageCount()const { return m_ImageCount; }
		VkRenderPass GetRenderPass()const { return m_RenderPass; }
	
		uint32_t AcquireImage(VulkanSemaphore* signalSemaphore, VulkanFence* signalFence, uint64_t timeout = UINT64_MAX)const;

		void Resize(uint32_t width,uint32_t height);


	private:
		VkSurfaceFormatKHR ChooseSurfaceFormat(VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice, VkSurfaceFormatKHR wantedFormat);
	
		VkPresentModeKHR ChoosePresentMode(VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice, VkPresentModeKHR wantedMode);
		VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t wantedWidth, uint32_t wantedHeight);
	
	
		void MakeFrames(VulkanLogicalDevice& device);
	private:
		VkSwapchainKHR m_Swapchain;
		std::vector<SwapchainFrame> m_Frames;
		VkSurfaceFormatKHR m_Format;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;
		uint32_t m_ImageCount;
		VkRenderPass m_RenderPass;
		VkSurfaceKHR m_Surface;
		std::vector<uint32_t> m_QueueFamilyIndices;
	};
	
}

