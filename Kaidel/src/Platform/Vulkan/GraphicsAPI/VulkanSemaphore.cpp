#include "KDpch.h"
#include "VulkanSemaphore.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanSemaphore::VulkanSemaphore()
	{
		VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VK_ASSERT(vkCreateSemaphore(VK_DEVICE.GetDevice(), &semaphoreInfo, nullptr, &m_Semaphore));
	}
	VulkanSemaphore::~VulkanSemaphore() {
		vkDestroySemaphore(VK_DEVICE.GetDevice(), m_Semaphore, nullptr);
	}
}
