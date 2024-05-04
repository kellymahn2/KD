#pragma once


#include "VulkanBase.h"
#include "VulkanSyncObjects.h"

namespace Kaidel {
	namespace Vulkan {
		enum class ImageSharingMode {
			NoShare,
			Share
		};

		struct InstanceSpecification {
			std::vector<const char*> WantedInstanceExtensions;
			std::vector<const char*> WantedInstanceLayers;
			std::string ApplicationName;
		};

		struct PhysicalDeviceSpecification {
			std::vector<const char*> WantedPhysicalDeviceExtensions;
		};

		struct QueueFamilyIndices {
			std::optional<uint32_t> GraphicsQueueFamilyIndex, PresentQueueFamilyIndex, TransferQueueFamilyIndex, ComputeQueueFamilyIndex;
			bool Valid() const {
				return GraphicsQueueFamilyIndex.has_value() && PresentQueueFamilyIndex.has_value();
			}
			operator bool()const {
				return Valid();
			}
		};

		struct DeviceQueues {
			VkQueue GraphicsQueue = VK_NULL_HANDLE;
			VkQueue PresentQueue = VK_NULL_HANDLE;
			VkQueue TransferQueue = VK_NULL_HANDLE;
			VkQueue ComputeQueue = VK_NULL_HANDLE;
		};

		struct LogicalDeviceSpecification {
			std::vector<const char*> Extensions;
			std::vector<const char*> Layers;
			VkPhysicalDeviceFeatures Features;
		};




		struct VulkanSwapchainSpecification {
			VkDevice LogicalDevice;
			VkPhysicalDevice PhysicalDevice;
			VkSurfaceKHR Surface;
			ImageSharingMode SharingMode;
			VkQueue PresentQueue;
			std::vector<VkSemaphore> WaitSemaphores;

			VkSemaphore RenderFinishedSemaphore;
			VkFence InFlightFence;
			VkSemaphore ImageAvailableSemaphore;

			VkSurfaceFormatKHR SwapchainFormat;
			VkPresentModeKHR SwapchainPresentMode;
			VkExtent2D Extent;
			uint32_t ImageCount = (uint32_t)(-1);
			std::vector<uint32_t> QueueFamiliesToShare;
			VkRenderPass RenderPass;

			VkCommandPool CommandPool;

		};


		struct SwapchainFrame {
			VkImage FrameImage;
			VkImageView ImageView;
			VkFramebuffer Framebuffer;
			VkCommandBuffer CommandBuffer;
			Ref<VulkanFence> InFlightFence;
			Ref<VulkanSemaphore> ImageAvailable, RenderFinished;
		};

	}
}
