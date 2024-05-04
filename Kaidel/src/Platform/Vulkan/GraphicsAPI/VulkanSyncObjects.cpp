#include "KDpch.h"
#include "VulkanSyncObjects.h"


namespace Kaidel {
	namespace Vulkan {
		VulkanFence::VulkanFence(VkDevice device,bool open)
			:m_LogicalDevice(device)
		{
			VK_STRUCT(VkFenceCreateInfo ,fenceInfo,FENCE_CREATE_INFO);
			if (open)
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			open = m_Open;
			VK_ASSERT(vkCreateFence(m_LogicalDevice, &fenceInfo, VK_ALLOCATOR_PTR, &m_Fence));
		}
		VulkanFence::~VulkanFence()
		{
			vkDestroyFence(m_LogicalDevice, m_Fence, VK_ALLOCATOR_PTR);
		}
		void VulkanFence::Wait()
		{
			Wait(UINT64_MAX);
		}
		void VulkanFence::Wait(uint64_t timeout)
		{
			if (m_Open)
				return;
			VkResult result = vkWaitForFences(m_LogicalDevice, 1, &m_Fence, VK_TRUE, timeout);
			if(result == VK_SUCCESS)
				m_Open = true;
		}
		void VulkanFence::Reset()
		{
			auto result = vkResetFences(m_LogicalDevice, 1, &m_Fence);
			m_Open = false;

		}
		VulkanSemaphore::VulkanSemaphore(VkDevice device)
			:m_LogicalDevice(device)
		{
			VK_STRUCT(VkSemaphoreCreateInfo, semaphoreInfo, SEMAPHORE_CREATE_INFO);
			VK_ASSERT(vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, VK_ALLOCATOR_PTR, &m_Semaphore));
			
		}
		VulkanSemaphore::~VulkanSemaphore()
		{
			vkDestroySemaphore(m_LogicalDevice, m_Semaphore, VK_ALLOCATOR_PTR);
		}
	}
}
