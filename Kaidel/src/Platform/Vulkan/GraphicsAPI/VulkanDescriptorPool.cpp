#include "KDpch.h"
#include "VulkanDescriptorPool.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanDescriptorPool::VulkanDescriptorPool(const std::vector<VkDescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags)
	{
		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = flags;
		poolInfo.maxSets = maxSetCount;
		poolInfo.poolSizeCount = (uint32_t)sizes.size();
		poolInfo.pPoolSizes = sizes.data();

		VK_ASSERT(vkCreateDescriptorPool(VK_DEVICE.GetDevice(), &poolInfo, nullptr, &m_Pool));
	}
	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		vkDestroyDescriptorPool(VK_DEVICE.GetDevice(), m_Pool, nullptr);
	}
}
