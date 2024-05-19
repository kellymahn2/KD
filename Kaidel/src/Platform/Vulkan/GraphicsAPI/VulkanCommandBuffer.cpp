#include "KDpch.h"
#include "VulkanCommandBuffer.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Vulkan {
		VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanCommandPool> pool, bool primary)
			:m_CommandBuffer(VK_NULL_HANDLE),m_CommandPool(pool)
		{
			VK_STRUCT(VkCommandBufferAllocateInfo, commandBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
			commandBufferInfo.commandPool = pool->GetCommandPool();
			commandBufferInfo.commandBufferCount = 1;
			commandBufferInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

			VK_ASSERT(vkAllocateCommandBuffers(VK_DEVICE, &commandBufferInfo, &m_CommandBuffer));
		}
	}

}
