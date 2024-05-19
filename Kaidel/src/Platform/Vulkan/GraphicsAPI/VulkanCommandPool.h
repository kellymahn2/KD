#pragma once

#include "VulkanBase.h"
#include "VulkanDefinitions.h"

namespace Kaidel {
	namespace Vulkan {

		class VulkanCommandPool : public IRCCounter<false> {
		public:
			VulkanCommandPool(uint32_t queueFamilyIndex,bool singleResetable = true);
			~VulkanCommandPool();

			VkCommandPool GetCommandPool()const { return m_CommandPool; }

		private:
			VkCommandPool m_CommandPool;
			uint32_t m_QueueFamilyIndex;
		};

	}
}
