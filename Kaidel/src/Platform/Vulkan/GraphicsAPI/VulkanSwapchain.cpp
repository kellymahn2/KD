#include "KDpch.h"
#include "VulkanSwapchain.h"
#include "VulkanGraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Kaidel {
	VulkanSwapchain::VulkanSwapchain(VkSurfaceKHR surface, VkSurfaceFormatKHR wantedSurfaceFormat, 
										VkPresentModeKHR wantedPresentMode, uint32_t wantedWidth, uint32_t wantedHeight, uint32_t wantedImageCount, 
										const std::vector<uint32_t>& queueFamilyIndices)
		:m_Swapchain(VK_NULL_HANDLE), m_Surface(surface), m_QueueFamilyIndices(queueFamilyIndices)
	{
	
		VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(surface,VK_PHYSICAL_DEVICE,wantedSurfaceFormat);
		VkPresentModeKHR presentMode = ChoosePresentMode(surface, VK_PHYSICAL_DEVICE,wantedPresentMode);

		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_PHYSICAL_DEVICE.GetDevice(), surface, &surfaceCapabilities));

		VkExtent2D extent = ChooseExtent(surfaceCapabilities, wantedWidth, wantedHeight);

		uint32_t imageCount = std::clamp(wantedImageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		
		VkSwapchainCreateInfoKHR swapchainInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.imageFormat = surfaceFormat.format;
		swapchainInfo.imageSharingMode = m_QueueFamilyIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.pQueueFamilyIndices = m_QueueFamilyIndices.data();
		swapchainInfo.queueFamilyIndexCount = (uint32_t)m_QueueFamilyIndices.size();
		swapchainInfo.presentMode = presentMode;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.surface = surface;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VK_ASSERT(vkCreateSwapchainKHR(VK_DEVICE.GetDevice(), &swapchainInfo, nullptr, &m_Swapchain));

		m_Frames.resize(imageCount);

		m_Format = surfaceFormat;
		m_PresentMode = presentMode;
		m_Extent = extent;
		m_ImageCount = imageCount;

		VkAttachmentDescription attachmentDesc{};
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.format = m_Format.format;
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc{};
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &attachmentRef;
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &attachmentDesc;
		passInfo.pSubpasses = &subpassDesc;
		passInfo.subpassCount = 1;

		VK_ASSERT(vkCreateRenderPass(VK_DEVICE.GetDevice(), &passInfo, nullptr, &m_RenderPass));

		MakeFrames(VK_DEVICE);
	}

	VulkanSwapchain::~VulkanSwapchain() {
		for (auto& frame : m_Frames) {
			vkDestroyFramebuffer(VK_DEVICE.GetDevice(), frame.Framebuffer, nullptr);
			vkDestroyImageView(VK_DEVICE.GetDevice(), frame.ImageView, nullptr);
		}

		vkDestroyRenderPass(VK_DEVICE.GetDevice(), m_RenderPass, nullptr);

		vkDestroySwapchainKHR(VK_DEVICE.GetDevice(), m_Swapchain, nullptr);
	}

	uint32_t VulkanSwapchain::AcquireImage(VulkanSemaphore* signalSemaphore, VulkanFence* signalFence, uint64_t timeout) const {
		uint32_t imageIndex = 0;
		VK_ASSERT(vkAcquireNextImageKHR(VK_DEVICE.GetDevice(), m_Swapchain, timeout, signalSemaphore ? signalSemaphore->GetSemaphore() : VK_NULL_HANDLE
			, signalFence ? signalFence->GetFence() : VK_NULL_HANDLE, &imageIndex));
		return imageIndex;
	}

	void VulkanSwapchain::Resize(uint32_t width,uint32_t height)
	{

		for (auto& frame : m_Frames) {
			vkDestroyFramebuffer(VK_DEVICE.GetDevice(), frame.Framebuffer, nullptr);
			vkDestroyImageView(VK_DEVICE.GetDevice(), frame.ImageView, nullptr);
		}

		vkDestroySwapchainKHR(VK_DEVICE.GetDevice(), m_Swapchain, nullptr);


		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_PHYSICAL_DEVICE.GetDevice(), m_Surface, &surfaceCapabilities));

		VkExtent2D extent = ChooseExtent(surfaceCapabilities, width, height);

		VkSwapchainCreateInfoKHR swapchainInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageColorSpace = m_Format.colorSpace;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.imageFormat = m_Format.format;
		swapchainInfo.imageSharingMode = m_QueueFamilyIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.minImageCount = m_ImageCount;
		swapchainInfo.pQueueFamilyIndices = m_QueueFamilyIndices.data();
		swapchainInfo.queueFamilyIndexCount = (uint32_t)m_QueueFamilyIndices.size();
		swapchainInfo.presentMode = m_PresentMode;
		swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainInfo.surface = m_Surface;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VkSwapchainKHR swapchain{};
		VK_ASSERT(vkCreateSwapchainKHR(VK_DEVICE.GetDevice(), &swapchainInfo, nullptr, &swapchain));

		

		m_Swapchain = swapchain;

		m_Extent = extent;

		MakeFrames(VK_DEVICE);
	}


	VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice, VkSurfaceFormatKHR wantedFormat) {
		uint32_t formatCount = 0;
		std::vector<VkSurfaceFormatKHR> formats;
		VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetDevice(), surface, &formatCount, nullptr));
		formats.resize(formatCount);
		VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.GetDevice(), surface, &formatCount, formats.data()));

		for (auto& format : formats) {
			if (format.format == wantedFormat.format && format.colorSpace == wantedFormat.colorSpace)
				return format;
		}
		return formats[0];
	}

	VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(VkSurfaceKHR surface,VulkanPhysicalDevice& physicalDevice, VkPresentModeKHR wantedMode) {
		uint32_t modeCount = 0;
		std::vector<VkPresentModeKHR> modes;
		VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.GetDevice(), surface, &modeCount, nullptr));
		modes.resize(modeCount);
		VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.GetDevice(), surface, &modeCount, modes.data()));

		for (auto& mode : modes) {
			if (mode == wantedMode)
				return wantedMode;
		}

		return modes[0];
	}


	VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t wantedWidth, uint32_t wantedHeight) {

		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		return { std::clamp(wantedWidth,capabilities.minImageExtent.width,capabilities.maxImageExtent.width),
				std::clamp(wantedHeight,capabilities.minImageExtent.height,capabilities.maxImageExtent.height)
		};
	}

	void VulkanSwapchain::MakeFrames(VulkanLogicalDevice& device) {

		uint32_t imageCount = 0;
		std::vector<VkImage> images;
		VK_ASSERT(vkGetSwapchainImagesKHR(device.GetDevice(), m_Swapchain, &imageCount, nullptr));
		images.resize(imageCount);
		VK_ASSERT(vkGetSwapchainImagesKHR(device.GetDevice(), m_Swapchain, &imageCount, images.data()));
		KD_CORE_ASSERT(imageCount == m_Frames.size());

		for (uint32_t i = 0; i < m_Frames.size(); ++i) {
			m_Frames[i].Image = images[i];

			VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			viewInfo.format = m_Format.format;
			viewInfo.image = images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			VkImageSubresourceRange range{};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = 0;
			range.baseMipLevel = 0;
			range.layerCount = 1;
			range.levelCount = 1;
			viewInfo.subresourceRange = range;

			VK_ASSERT(vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &m_Frames[i].ImageView));


			VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.width = m_Extent.width;
			framebufferInfo.height = m_Extent.height;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.layers = 1;
			framebufferInfo.pAttachments = &m_Frames[i].ImageView;

			VK_ASSERT(vkCreateFramebuffer(device.GetDevice(), &framebufferInfo, nullptr, &m_Frames[i].Framebuffer));
		}
	}
}
