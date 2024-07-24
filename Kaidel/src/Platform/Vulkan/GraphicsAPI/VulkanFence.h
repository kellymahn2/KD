#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanLogicalDevice.h"

namespace Kaidel {
	class VulkanFence : public IRCCounter<false> {
	public:
		VulkanFence(VkFenceCreateFlags flags);

		~VulkanFence();

		VkFence GetFence()const { return m_Fence; }

		void Wait(uint64_t timeout = UINT64_MAX)const;
		void Reset()const;

	private:
		VkFence m_Fence;
	};
}
