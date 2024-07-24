#include "KDpch.h"
#include "VulkanFence.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	
	VulkanFence::VulkanFence(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = flags;
		VK_ASSERT(vkCreateFence(VK_DEVICE.GetDevice(), &fenceInfo, nullptr, &m_Fence));
	}
	VulkanFence::~VulkanFence()
	{
		vkDestroyFence(VK_DEVICE.GetDevice(), m_Fence, nullptr);
	}
	void VulkanFence::Wait(uint64_t timeout) const {
		VK_ASSERT(vkWaitForFences(VK_DEVICE.GetDevice(), 1, &m_Fence, VK_TRUE, timeout));
	}

	void VulkanFence::Reset() const {
		vkResetFences(VK_DEVICE.GetDevice(), 1, &m_Fence);
	}
}
