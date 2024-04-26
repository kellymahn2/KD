#include "KDpch.h"
#include "VulkanSwapchain.h"


namespace Kaidel {
	namespace Vulkan {


		namespace Utils {
			struct SwapchainSupportDetails {
				VkSurfaceCapabilitiesKHR Capabilities;
				std::vector<VkSurfaceFormatKHR> SupportedFormats;
				std::vector<VkPresentModeKHR> SupportedPresentModes;
			};

			static SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
				SwapchainSupportDetails support{};

				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &support.Capabilities);


				std::vector<VkSurfaceFormatKHR> surfaceFormats;
				uint32_t surfaceFormatCount = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, nullptr);
				surfaceFormats.resize(surfaceFormatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCount, surfaceFormats.data());

				std::vector<VkPresentModeKHR> presentModes;
				uint32_t presentModeCount = 0;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
				presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());

				support.SupportedFormats = std::move(surfaceFormats);
				support.SupportedPresentModes = std::move(presentModes);

				return support;
			}

			static VkSurfaceFormatKHR ChooseSwapchainFormat(VkSurfaceFormatKHR wantedFormat,const std::vector<VkSurfaceFormatKHR>& formats) {
				for (auto& format : formats) {
					if ((format.format == wantedFormat.format) && (format.colorSpace == wantedFormat.colorSpace)) {
						return format;
					}
				}
				return formats[0];
			}

			static VkPresentModeKHR ChooseSwapchainPresentMode(VkPresentModeKHR wantedPresentMode,const std::vector<VkPresentModeKHR>& presentModes) {
				for (auto& presentMode : presentModes) {
					if (presentMode == wantedPresentMode)
						return presentMode;
				}
				return VK_PRESENT_MODE_FIFO_KHR;
			}

			static VkExtent2D ChooseSwapchainSize(uint32_t width, uint32_t height, const VkSurfaceCapabilitiesKHR& capabilities) {
				if (capabilities.currentExtent.width != UINT32_MAX) {
					return capabilities.currentExtent;
				}
				else {
					VkExtent2D actualExtent = { width, height };
					actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
					actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
					return actualExtent;
				}
			}

			static uint32_t ChooseImageCount(uint32_t wantedImageCount, const VkSurfaceCapabilitiesKHR& capabilities) {
				if (capabilities.minImageCount <= wantedImageCount && capabilities.maxImageCount >= wantedImageCount)
					return wantedImageCount;
				return std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
			}

			static VkSharingMode KaidelSharingModeToVulkanSharingMode(ImageSharingMode mode) {

				switch (mode)
				{
				case Kaidel::Vulkan::ImageSharingMode::NoShare:return VK_SHARING_MODE_EXCLUSIVE;
				case Kaidel::Vulkan::ImageSharingMode::Share: return VK_SHARING_MODE_CONCURRENT;
				}
				return VK_SHARING_MODE_MAX_ENUM;
			}

			static std::vector<VkImage> GetSwapchainImages(VkDevice logicalDevice, VkSwapchainKHR swapchain) {
				std::vector<VkImage> swapchainImages;
				uint32_t swapchainImageCount = 0;
				vkGetSwapchainImagesKHR(logicalDevice,swapchain, &swapchainImageCount, nullptr);
				swapchainImages.resize(swapchainImageCount);
				vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, swapchainImages.data());
				return swapchainImages;
			}

			struct SwapchainCreateResult {
				std::vector<SwapchainFrame> Frames;
				VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
			};

			static SwapchainCreateResult CreateSwapchain(VulkanSwapchainSpecification& specification) {
				SwapchainSupportDetails supportDetails = QuerySwapchainSupportDetails(specification.PhysicalDevice, specification.Surface);
				
				VkSurfaceFormatKHR format = ChooseSwapchainFormat(specification.SwapchainFormat, supportDetails.SupportedFormats);

				VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(specification.SwapchainPresentMode, supportDetails.SupportedPresentModes);

				VkExtent2D size = ChooseSwapchainSize(specification.Extent.width, specification.Extent.height, supportDetails.Capabilities);

				uint32_t imageCount = ChooseImageCount(specification.ImageCount, supportDetails.Capabilities);

				VK_STRUCT(VkSwapchainCreateInfoKHR, swapchainInfo, SWAPCHAIN_CREATE_INFO_KHR);
				swapchainInfo.clipped = VK_TRUE;
				swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapchainInfo.imageArrayLayers = 1;
				swapchainInfo.imageColorSpace = format.colorSpace;
				swapchainInfo.imageExtent = size;
				swapchainInfo.imageFormat = format.format;
				swapchainInfo.imageSharingMode = KaidelSharingModeToVulkanSharingMode(specification.SharingMode);
				swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				swapchainInfo.minImageCount = imageCount;
				swapchainInfo.pQueueFamilyIndices = specification.QueueFamiliesToShare.data();
				swapchainInfo.queueFamilyIndexCount = (uint32_t)specification.QueueFamiliesToShare.size();
				swapchainInfo.presentMode = presentMode;
				swapchainInfo.preTransform = supportDetails.Capabilities.currentTransform;
				swapchainInfo.surface = specification.Surface;

				SwapchainCreateResult result{};
				VK_ASSERT(vkCreateSwapchainKHR(specification.LogicalDevice, &swapchainInfo, VK_ALLOCATOR_PTR, &result.Swapchain));
				
				std::vector<VkImage> swapchainImages = GetSwapchainImages(specification.LogicalDevice, result.Swapchain);

				for (auto& image : swapchainImages) {
					SwapchainFrame frame{};
					frame.FrameImage = image;

					VkImageSubresourceRange subresourceRange{};
					subresourceRange.baseArrayLayer = 0;
					subresourceRange.baseMipLevel = 0;
					subresourceRange.layerCount = 1;
					subresourceRange.levelCount = 1;
					subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

					VkImageViewCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					createInfo.subresourceRange = subresourceRange;
					createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
					createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
					createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
					createInfo.image = image;
					createInfo.format = format.format;

					VK_ASSERT(vkCreateImageView(specification.LogicalDevice, &createInfo, VK_ALLOCATOR_PTR, &frame.ImageView));

					result.Frames.push_back(frame);
				}

				specification.SwapchainFormat = format;
				specification.SwapchainPresentMode = presentMode;
				specification.Extent = size;
				specification.ImageCount = imageCount;

				return  result;
			}
		}

		VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainSpecification& specification)
			:m_Specification(specification)
		{
			Invalidate();
		}

		VulkanSwapchain::~VulkanSwapchain()
		{
			if (m_Swapchain) {
				DestroyCurrentSwapchain();
			}
		}

		void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
		{
			if ((width * height == 0) ) {
				KD_CORE_WARN("Tried to resize swapchain to ({},{})", width, height);
				return;
			}

			if ((width == m_Specification.Extent.width && height == m_Specification.Extent.height)) {
				return;
			}

			m_Specification.Extent.width= width;
			m_Specification.Extent.height = height;
		}

		void VulkanSwapchain::Present(uint32_t imageIndex)
		{

		}

		uint32_t VulkanSwapchain::AcquireImage()
		{
			uint32_t imageIndex;
			VK_ASSERT(vkAcquireNextImageKHR(m_Specification.LogicalDevice, m_Swapchain, UINT64_MAX, m_Specification.ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
			return imageIndex;
		}

		void VulkanSwapchain::DestroyCurrentSwapchain()
		{
			for (auto& frame : m_Frames) {
				vkDestroyImageView(m_Specification.LogicalDevice, frame.ImageView, nullptr);
				vkDestroyFramebuffer(m_Specification.LogicalDevice, frame.Framebuffer, nullptr);
			}
			vkDestroySwapchainKHR(m_Specification.LogicalDevice, m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
			m_Frames.clear();
		}

		void VulkanSwapchain::Invalidate()
		{
			if (m_Swapchain) {
				DestroyCurrentSwapchain();
			}

			auto swapchainCreateResult = Utils::CreateSwapchain(m_Specification);

			m_Swapchain = swapchainCreateResult.Swapchain;
			m_Frames = swapchainCreateResult.Frames;
		}

	}
}
