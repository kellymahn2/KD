#pragma once
#include "VulkanBase.h"

namespace Kaidel {
	namespace Vulkan {


		class VulkanFence : public IRCCounter<false>{
		public:
			VulkanFence(VkDevice device,bool open = true);
			~VulkanFence();
			
			void Wait();
			void Wait(uint64_t timeout);
			void Reset();
			VkFence GetFence() const{ return m_Fence; }

		private:
			VkDevice m_LogicalDevice = VK_NULL_HANDLE;
			VkFence m_Fence = VK_NULL_HANDLE;
			bool m_Open = false;
		};


		class VulkanSemaphore :public IRCCounter<false>{
		public:
			VulkanSemaphore(VkDevice device);
			~VulkanSemaphore();

			VkSemaphore GetSemaphore()const { return m_Semaphore; }


		private:
			VkDevice m_LogicalDevice = VK_NULL_HANDLE;
			VkSemaphore m_Semaphore = VK_NULL_HANDLE;
		};



	}
}
