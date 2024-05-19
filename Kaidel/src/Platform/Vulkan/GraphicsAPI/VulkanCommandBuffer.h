#pragma once

#include "VulkanBase.h"
#include "VulkanCommandPool.h"


namespace Kaidel {
	namespace Vulkan {
		class VulkanCommandBuffer : public IRCCounter<false>{
		public:
			VulkanCommandBuffer(Ref<VulkanCommandPool> pool,bool primary = true);

			VkCommandBuffer GetCommandBuffer()const { return m_CommandBuffer; }
			Ref<VulkanCommandPool> GetCommandPool()const { return m_CommandPool; }

		private:
			VkCommandBuffer m_CommandBuffer;
			Ref<VulkanCommandPool> m_CommandPool;
		};
	}
}
