#include "KDpch.h"
#include "VulkanCommandBuffer.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Vulkan {
		VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanCommandPool> pool, bool primary)
			:m_CommandPool(pool)
		{
			VK_STRUCT(VkCommandBufferAllocateInfo, commandBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
			commandBufferInfo.commandPool = pool->GetCommandPool();
			commandBufferInfo.commandBufferCount = m_CommandBuffers.GetResources().size();
			commandBufferInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;


			VK_ASSERT(vkAllocateCommandBuffers(VK_DEVICE, &commandBufferInfo,m_CommandBuffers.begin()._Ptr));
		}
		void VulkanCommandBuffer::BeginRecording(uint32_t flags)
		{
			VK_STRUCT(VkCommandBufferBeginInfo, beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
			beginInfo.flags = flags;
			VK_ASSERT(vkBeginCommandBuffer(*m_CommandBuffers, &beginInfo));
		}
		void VulkanCommandBuffer::EndRecording()
		{
			VK_ASSERT(vkEndCommandBuffer(*m_CommandBuffers));
		}
		void VulkanCommandBuffer::Reset(uint32_t flags)
		{
			vkResetCommandBuffer(*m_CommandBuffers, flags);
		}
		void VulkanCommandBuffer::Submit(const CommandBufferSubmitSpecification& submitInfo)
		{
			VK_STRUCT(VkSubmitInfo, info, VK_STRUCTURE_TYPE_SUBMIT_INFO);

			info.commandBufferCount = 1;
			info.pCommandBuffers = &*m_CommandBuffers;
			info.pSignalSemaphores = submitInfo.SignalSemaphores.data();
			info.signalSemaphoreCount = (uint32_t)submitInfo.SignalSemaphores.size();
			info.pWaitSemaphores = submitInfo.WaitSemaphores.data();
			info.waitSemaphoreCount = (uint32_t)submitInfo.WaitSemaphores.size();
			info.pWaitDstStageMask = &submitInfo.WaitStageMask;

			VK_ASSERT(vkQueueSubmit(submitInfo.Queue, submitInfo.SubmitCount, &info, submitInfo.Fence));
		}
	}

}
