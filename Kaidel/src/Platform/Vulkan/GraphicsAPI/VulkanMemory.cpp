#include "KDpch.h"
#include "VulkanMemory.h"



namespace Kaidel {
	namespace Utils {


		uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, VkMemoryPropertyFlags requestedProperties) {
			VkPhysicalDeviceMemoryProperties props{};
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);

			for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
				bool supported = (supportedMemoryIndices & (1 << i));
				bool sufficient = (props.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

				if (supported && sufficient)
					return i;

			}

			return -1;
		}



		BufferCreateResult CreateBuffer(const BufferSpecification& specification) {
			BufferCreateResult result{};

			const auto& uniqueIndices = specification.QueueFamilies;

			VK_STRUCT(VkBufferCreateInfo, bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
			bufferInfo.pQueueFamilyIndices = uniqueIndices.data();
			bufferInfo.queueFamilyIndexCount = (uint32_t)uniqueIndices.size();
			bufferInfo.sharingMode = bufferInfo.queueFamilyIndexCount > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			bufferInfo.size = specification.Size;
			bufferInfo.usage = specification.BufferUsage;

			VK_ASSERT(vkCreateBuffer(specification.LogicalDevice, &bufferInfo, VK_ALLOCATOR_PTR, &result.Buffer));

			VkMemoryRequirements bufferMemReq{};
			vkGetBufferMemoryRequirements(specification.LogicalDevice, result.Buffer, &bufferMemReq);

			VK_STRUCT(VkMemoryAllocateInfo, memoryInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
			memoryInfo.allocationSize = bufferMemReq.size;
			memoryInfo.memoryTypeIndex = FindMemoryTypeIndex(specification.PhysicalDevice, bufferMemReq.memoryTypeBits, specification.MemoryPropertyFlags);
			VK_ASSERT(vkAllocateMemory(specification.LogicalDevice, &memoryInfo, VK_ALLOCATOR_PTR, &result.AllocatedMemory));


			VK_ASSERT(vkBindBufferMemory(specification.LogicalDevice, result.Buffer, result.AllocatedMemory, 0));

			return result;
		}

		ImageCreateResult CreateImage(const ImageSpecification& specification)
		{
			ImageCreateResult result{};


			VK_STRUCT(VkImageCreateInfo, imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
			imageInfo.arrayLayers = specification.ArrayLayerCount;
			imageInfo.extent = { specification.Width,specification.Height,specification.Depth };
			imageInfo.format = specification.Format;
			imageInfo.imageType = specification.ImageType;
			imageInfo.initialLayout = specification.InitialLayout;
			imageInfo.mipLevels = specification.MipLevels;
			imageInfo.pQueueFamilyIndices = specification.QueueFamilies.data();
			imageInfo.queueFamilyIndexCount = (uint32_t)specification.QueueFamilies.size();
			imageInfo.samples = specification.SampleCount;
			imageInfo.sharingMode = specification.QueueFamilies.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = specification.ImageUsage;

			VK_ASSERT(vkCreateImage(specification.LogicalDevice, &imageInfo, VK_ALLOCATOR_PTR, &result.Image));

			VkMemoryRequirements memReq{};
			vkGetImageMemoryRequirements(specification.LogicalDevice, result.Image, &memReq);

			VK_STRUCT(VkMemoryAllocateInfo, memoryInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
			memoryInfo.memoryTypeIndex = FindMemoryTypeIndex(specification.PhysicalDevice, memReq.memoryTypeBits, specification.MemoryPropertyFlags);
			memoryInfo.allocationSize = memReq.size;
			VK_ASSERT(vkAllocateMemory(specification.LogicalDevice, &memoryInfo, VK_ALLOCATOR_PTR, &result.AllocatedMemory));

			VK_ASSERT(vkBindImageMemory(specification.LogicalDevice, result.Image, result.AllocatedMemory,0));
			return result;
		}

		void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize copySize, VkDevice device, VkCommandBuffer commandBuffer, VkQueue transferQueue) {

			vkResetCommandBuffer(commandBuffer, 0);
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			VkBufferCopy copy{};
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = copySize;
			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copy);
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
			vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(transferQueue);
		}

		void DeleteBuffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory) {
			if (buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, buffer, VK_ALLOCATOR_PTR);
			if (memory != VK_NULL_HANDLE)
				vkFreeMemory(device, memory, VK_ALLOCATOR_PTR);
		}
	}

	namespace Vulkan {
	}
}
