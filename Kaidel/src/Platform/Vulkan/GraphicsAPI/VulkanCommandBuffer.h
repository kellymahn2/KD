#pragma once

#include "VulkanBase.h"
#include "VulkanCommandPool.h"

#include "PerFrameResource.h"


namespace Kaidel {
	namespace Vulkan {

		struct CommandBufferSubmitSpecification {
			VkQueue Queue;
			uint32_t SubmitCount;
			VkFlags WaitStageMask;
			VkFence Fence;
			std::vector<VkSemaphore> WaitSemaphores;
			std::vector<VkSemaphore> SignalSemaphores;


			CommandBufferSubmitSpecification(VkQueue queue, uint32_t submitCount = 1, VkFlags waitStageMask = 0, VkFence fence = VK_NULL_HANDLE,
								const std::vector<VkSemaphore>& waitSemaphores = {}, const std::vector<VkSemaphore>& signalSemaphores = {})
				:Queue(queue),SubmitCount(submitCount),WaitStageMask(waitStageMask), Fence(fence), WaitSemaphores(waitSemaphores), SignalSemaphores(signalSemaphores)
			{}
		};

		


		class VulkanCommandBuffer : public IRCCounter<false>{
		public:
			VulkanCommandBuffer(Ref<VulkanCommandPool> pool,bool primary = true);

			VkCommandBuffer GetCommandBuffer(){ return *m_CommandBuffers; }
			Ref<VulkanCommandPool> GetCommandPool()const { return m_CommandPool; }
			
			void BeginRecording(uint32_t flags);
			void EndRecording();
			void Reset(uint32_t flags);

			void Submit(const CommandBufferSubmitSpecification& submitInfo);

		private:
		private:

			Ref<VulkanCommandPool> m_CommandPool;

			PerFrameResource<VkCommandBuffer> m_CommandBuffers;

		};
	}
}
