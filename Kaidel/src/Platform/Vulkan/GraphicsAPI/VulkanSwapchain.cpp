#include "KDpch.h"
#include "VulkanSwapchain.h"
#include "VulkanGraphicsContext.h"


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

			static VkFramebuffer CreateSwapchainFramebuffer(VkDevice device,uint32_t width, uint32_t height,VkImageView attachment,VkRenderPass renderPass) {
				VkFramebuffer framebuffer = VK_NULL_HANDLE;

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = &attachment;
				framebufferInfo.width = width;
				framebufferInfo.height = height;
				framebufferInfo.layers = 1;
				VK_ASSERT(vkCreateFramebuffer(device , &framebufferInfo, nullptr, &framebuffer));
				return framebuffer;
			}

			static VkImageView CreateSwapchainImageView(VkDevice device,VkImage image,VkFormat format) {

				VkImageView imageView = VK_NULL_HANDLE;

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
				createInfo.format = format;
				VK_ASSERT(vkCreateImageView(device, &createInfo, VK_ALLOCATOR_PTR, &imageView));
				return imageView;
			}

			struct SwapchainCreateResult {
				std::vector<SwapchainFrame> Frames;
				VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
			};

			static SwapchainCreateResult CreateSwapchain(VulkanSwapchainSpecification& specification,VkRenderPass renderPass) {
				SwapchainSupportDetails supportDetails = QuerySwapchainSupportDetails(specification.PhysicalDevice, specification.Surface);
				
				VkSurfaceFormatKHR format = ChooseSwapchainFormat(specification.SwapchainFormat, supportDetails.SupportedFormats);

				VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(specification.SwapchainPresentMode, supportDetails.SupportedPresentModes);

				VkExtent2D size = ChooseSwapchainSize(specification.Extent.width, specification.Extent.height, supportDetails.Capabilities);

				uint32_t imageCount = ChooseImageCount(specification.ImageCount, supportDetails.Capabilities);

				VK_STRUCT(VkSwapchainCreateInfoKHR, swapchainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
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

					//Image view
					frame.ImageView = CreateSwapchainImageView(specification.LogicalDevice, image, format.format);
					
					//Framebuffer
					frame.Framebuffer = CreateSwapchainFramebuffer(specification.LogicalDevice, size.width, size.height, frame.ImageView, renderPass);

					//Sync objects
					frame.ImageAvailable = CreateRef<VulkanSemaphore>(specification.LogicalDevice);
					frame.RenderFinished = CreateRef<VulkanSemaphore>(specification.LogicalDevice);
					frame.InFlightFence = CreateRef<VulkanFence>(specification.LogicalDevice, true);

					VkCommandBufferAllocateInfo allocInfo{};
					allocInfo.commandPool = specification.CommandPool;
					allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
					allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
					allocInfo.commandBufferCount = 1;

					VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
					VK_ASSERT(vkAllocateCommandBuffers(specification.LogicalDevice, &allocInfo, &commandBuffer));

					frame.CommandBuffer = commandBuffer;
					result.Frames.push_back(frame);
				}

				specification.SwapchainFormat = format;
				specification.SwapchainPresentMode = presentMode;
				specification.Extent = size;
				specification.ImageCount = imageCount;

				return  result;
			}

			static VkRenderPass CreateSwapchainRenderPass(VkDevice logicalDevice, VkSurfaceFormatKHR format) {

				VkRenderPass renderPass = VK_NULL_HANDLE;

				// Color attachment description
				VkAttachmentDescription colorAttachment = {};
				colorAttachment.format = format.format;
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				// Attachment reference
				VkAttachmentReference colorAttachmentRef = {};
				colorAttachmentRef.attachment = 0;
				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				// Subpass description
				VkSubpassDescription subpass = {};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &colorAttachmentRef;

				// Subpass dependency
				VkSubpassDependency dependency = {};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				// Render pass create info
				VkRenderPassCreateInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = 1;
				renderPassInfo.pAttachments = &colorAttachment;
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = 1;
				renderPassInfo.pDependencies = &dependency;

				VK_ASSERT(vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass));
				return renderPass;
			}

		}

		VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainSpecification& specification)
			:m_Specification(specification)
		{
			
			VkSurfaceFormatKHR format{};
			{
				std::vector<VkSurfaceFormatKHR> surfaceFormats;
				uint32_t surfaceFormatCount = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(specification.PhysicalDevice, specification.Surface, &surfaceFormatCount, nullptr);
				surfaceFormats.resize(surfaceFormatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(specification.PhysicalDevice, specification.Surface, &surfaceFormatCount, surfaceFormats.data());

				format = Utils::ChooseSwapchainFormat(specification.SwapchainFormat, surfaceFormats);
			}
			m_RenderPass = Utils::CreateSwapchainRenderPass(specification.LogicalDevice, format);

			Invalidate();
		}

		VulkanSwapchain::~VulkanSwapchain()
		{
			if (m_Swapchain) {
				DestroyCurrentSwapchain();
			}

			vkDestroyRenderPass(m_Specification.LogicalDevice, m_RenderPass, VK_ALLOCATOR_PTR);
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
			Invalidate();
		}

		void VulkanSwapchain::Present(uint32_t imageIndex)
		{
			VkSemaphore waitSemaphores[] = {m_Frames[imageIndex].RenderFinished->GetSemaphore()};
			// Present the acquired image
			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = waitSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &m_Swapchain;
			presentInfo.pImageIndices = &imageIndex;
			auto res = vkQueuePresentKHR(m_Specification.PresentQueue, &presentInfo);
			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
				Invalidate();
			}
		}

		uint32_t VulkanSwapchain::AcquireImage(uint32_t lastImageIndex)
		{
			
			KD_CORE_ASSERT(lastImageIndex >= 0 && lastImageIndex < m_Frames.size(), "At least one image should be acquired");

			m_Frames[lastImageIndex].InFlightFence->Wait();
			m_Frames[lastImageIndex].InFlightFence->Reset();

			uint32_t imageIndex;
			VK_ASSERT(vkAcquireNextImageKHR(m_Specification.LogicalDevice, m_Swapchain, UINT64_MAX, m_Frames[lastImageIndex].ImageAvailable->GetSemaphore(), VK_NULL_HANDLE, &imageIndex));
			m_LastAcquiredImage = imageIndex;
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
			vkDeviceWaitIdle(m_Specification.LogicalDevice);
			if (m_Swapchain) {
				DestroyCurrentSwapchain();
			}

			auto swapchainCreateResult = Utils::CreateSwapchain(m_Specification,m_RenderPass);

			m_Swapchain = swapchainCreateResult.Swapchain;
			m_Frames = swapchainCreateResult.Frames;

		}

	}
}
