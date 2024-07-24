#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanLogicalDevice.h"



namespace Kaidel {
	class VulkanSemaphore : public IRCCounter<false>{
	public:
		VulkanSemaphore();

		~VulkanSemaphore();

		VkSemaphore GetSemaphore()const { return m_Semaphore; }


	private:
		VkSemaphore m_Semaphore;
	};

}
