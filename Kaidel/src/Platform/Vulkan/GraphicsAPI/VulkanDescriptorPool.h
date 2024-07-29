#pragma once

#include "Kaidel/Renderer/GraphicsAPI/DescriptorPool.h"
#include "Platform/Vulkan/VulkanDefinitions.h"
#include <glad/vulkan.h>

namespace Kaidel {
	class VulkanDescriptorPool : public DescriptorPool{
	public:
		VulkanDescriptorPool(const std::vector<DescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags);
		~VulkanDescriptorPool();

		VkDescriptorPool GetDescriptorPool()const { return m_Pool; }
	private:
		VkDescriptorPool m_Pool;

	};
}
