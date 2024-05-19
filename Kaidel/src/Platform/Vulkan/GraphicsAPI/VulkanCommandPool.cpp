#include "KDpch.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	namespace Vulkan {

		VulkanCommandPool::VulkanCommandPool(uint32_t queueFamilyIndex, bool singleResetable)
			:m_CommandPool(VK_NULL_HANDLE),m_QueueFamilyIndex(queueFamilyIndex)
		{
			VK_STRUCT(VkCommandPoolCreateInfo, commandPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
			commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

			commandPoolInfo.flags = singleResetable ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0;


			VK_ASSERT(vkCreateCommandPool(VK_DEVICE, &commandPoolInfo, VK_ALLOCATOR_PTR, &m_CommandPool));
		}

		VulkanCommandPool::~VulkanCommandPool()
		{
			vkDestroyCommandPool(VK_DEVICE, m_CommandPool, VK_ALLOCATOR_PTR);
		}
	}
}
