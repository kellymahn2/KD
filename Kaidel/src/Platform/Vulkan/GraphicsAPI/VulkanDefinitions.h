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
				return GraphicsQueueFamilyIndex.has_value() && PresentQueueFamilyIndex.has_value() && TransferQueueFamilyIndex.has_value();
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
			VkPhysicalDeviceFeatures2 Features2;
		};




		struct VulkanSwapchainSpecification {
			VkDevice LogicalDevice;
			VkPhysicalDevice PhysicalDevice;
			VkSurfaceKHR Surface;
			ImageSharingMode SharingMode;
			VkQueue PresentQueue;

			VkSurfaceFormatKHR SwapchainFormat;
			VkPresentModeKHR SwapchainPresentMode;
			VkExtent2D Extent;
			uint32_t ImageCount = (uint32_t)(-1);
			std::vector<uint32_t> QueueFamiliesToShare;

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


		struct VulkanBuffer {
			VkBuffer Buffer;
			VkDeviceMemory DeviceMemory;
			uint32_t Size;
			void* MappedMemory = nullptr;

			VulkanBuffer() = default;

			VulkanBuffer(VkBuffer buffer, VkDeviceMemory deviceMemory, uint32_t size)
				:Buffer(buffer),DeviceMemory(deviceMemory),Size(size)
			{}

			void Map(VkDevice device,uint32_t size = -1,uint32_t offset = 0,VkMemoryMapFlags flags = 0) {
				if (size == -1)
					size = Size;

				VK_ASSERT(vkMapMemory(device, DeviceMemory, offset, size, flags, &MappedMemory));
			}
			void Unmap(VkDevice device) {
				vkUnmapMemory(device, DeviceMemory);
			}

		};


		enum class CommandBufferState {
			Initial,
			Recording,
			Pending
		};


	}
}
