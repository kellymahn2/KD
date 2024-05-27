#pragma once
#include "VulkanBase.h"
#include "VulkanDefinitions.h"


namespace Kaidel {
	namespace Utils {
		struct BufferCreateResult {
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceMemory AllocatedMemory = VK_NULL_HANDLE;
		};
		struct BufferSpecification {
			VkDevice LogicalDevice;
			VkPhysicalDevice PhysicalDevice;
			VkMemoryPropertyFlags MemoryPropertyFlags;
			VkBufferUsageFlags BufferUsage;
			uint32_t Size;
			std::vector<uint32_t> QueueFamilies;
		};

		struct ImageCreateResult {
			VkImage Image = VK_NULL_HANDLE;
			VkDeviceMemory AllocatedMemory = VK_NULL_HANDLE;
		};

		struct ImageSpecification {
			VkDevice LogicalDevice;
			VkPhysicalDevice PhysicalDevice;
			VkMemoryPropertyFlags MemoryPropertyFlags;
			VkFormat Format;
			uint32_t Width;
			uint32_t Height;
			uint32_t Depth;
			VkImageUsageFlags ImageUsage;
			uint32_t ArrayLayerCount;
			uint32_t MipLevels;
			VkSampleCountFlagBits SampleCount;
			VkImageLayout InitialLayout;
			VkImageType ImageType;
			std::vector<uint32_t> QueueFamilies;
		};

		uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, VkMemoryPropertyFlags requestedProperties);
		BufferCreateResult CreateBuffer(const BufferSpecification& specification);
		ImageCreateResult CreateImage(const ImageSpecification& specification);
		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize copySize, VkDevice device, VkCommandBuffer commandBuffer, VkQueue transferQueue);
		void DeleteBuffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory);
	}
	namespace Vulkan {
		
	}
}
