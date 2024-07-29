#include "KDpch.h"
#include "VulkanDescriptorPool.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanDescriptorPool::VulkanDescriptorPool(const std::vector<DescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags)
	{
		std::vector<VkDescriptorPoolSize> poolSizes{};
		for (auto& size : sizes) {
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = size.DescriptorCount;
			poolSize.type = Utils::DescriptorTypeToVulkanDescriptorType(size.Type);
			poolSizes.push_back(poolSize);
		}


		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = flags;
		poolInfo.maxSets = maxSetCount;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();

		VK_ASSERT(vkCreateDescriptorPool(VK_DEVICE.GetDevice(), &poolInfo, nullptr, &m_Pool));
	}
	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		vkDestroyDescriptorPool(VK_DEVICE.GetDevice(), m_Pool, nullptr);
	}
}
