#include "KDpch.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {


	namespace Utils {

		static uint32_t CommandPoolOperationTypeToQueueIndex(CommandPoolOperationType opType) {
			switch (opType)
			{
			case Kaidel::CommandPoolOperationType::Graphics:return VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue").FamilyIndex;
			case Kaidel::CommandPoolOperationType::Compute:return VK_PHYSICAL_DEVICE.GetQueue("ComputeQueue").FamilyIndex;
			case Kaidel::CommandPoolOperationType::Transfer:return VK_PHYSICAL_DEVICE.GetQueue("TransferQueue").FamilyIndex;
			}

			KD_CORE_ASSERT(false, "Unknown operation");
			return 0;
		}


		static VkCommandPoolCreateFlags ExtractCommandPoolFlags(CommandPoolFlags flags) {
			VkCommandPoolCreateFlags result = 0;

			if (flags & CommandPoolFlags_CommandBufferReset) {
				result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			}
			if (flags & CommandPoolFlags_Transient) {
				result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			}

			return result;
		}
	}



	VulkanCommandPool::VulkanCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = flags;
		VK_ASSERT(vkCreateCommandPool(VK_DEVICE.GetDevice(), &poolInfo, nullptr, &m_CommandPool));
	}
	VulkanCommandPool::VulkanCommandPool(CommandPoolOperationType opType, CommandPoolFlags flags)
	{
		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = Utils::CommandPoolOperationTypeToQueueIndex(opType);
		poolInfo.flags = Utils::ExtractCommandPoolFlags(flags);
		VK_ASSERT(vkCreateCommandPool(VK_DEVICE.GetDevice(), &poolInfo, nullptr, &m_CommandPool));
	}
	VulkanCommandPool::~VulkanCommandPool()
	{
		vkDestroyCommandPool(VK_DEVICE.GetDevice(), m_CommandPool, nullptr);
	}

	VkCommandBuffer VulkanCommandPool::BeginSingleTimeCommands(VkCommandBufferUsageFlags flags)
	{
		VkCommandBuffer commandBuffer{};
		VkCommandBufferAllocateInfo bufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		bufferInfo.commandBufferCount = 1;
		bufferInfo.commandPool = m_CommandPool;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(VK_DEVICE.GetDevice(), &bufferInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = flags;
		VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		return commandBuffer;
	}

	void VulkanCommandPool::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkFence signalFence)
	{
		VK_ASSERT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, signalFence));

		VK_ASSERT(vkWaitForFences(VK_DEVICE.GetDevice(), 1, &signalFence, VK_TRUE, UINT64_MAX));
		VK_ASSERT(vkResetFences(VK_DEVICE.GetDevice(), 1, &signalFence));

		vkFreeCommandBuffers(VK_DEVICE.GetDevice(), m_CommandPool, 1, &commandBuffer);
	}

	void VulkanCommandPool::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) 
	{
		VK_ASSERT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;


		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(VK_DEVICE.GetDevice(), m_CommandPool, 1, &commandBuffer);
	}

	void VulkanCommandPool::Reset(int flags)
	{
		VK_ASSERT(vkResetCommandPool(VK_DEVICE.GetDevice(), m_CommandPool, flags));
	}

}
