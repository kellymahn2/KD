#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"

#include <glad/vulkan.h>

namespace Kaidel {
	class VulkanDescriptorPool {
	public:
		VulkanDescriptorPool(const std::vector<VkDescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags);
		~VulkanDescriptorPool();

		VkDescriptorPool GetDescriptorPool()const { return m_Pool; }

	private:
		VkDescriptorPool m_Pool;

	};
}
