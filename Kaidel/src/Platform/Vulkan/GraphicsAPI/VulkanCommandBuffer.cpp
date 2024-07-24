#include "KDpch.h"
#include "VulkanCommandBuffer.h"

#include "VulkanGraphicsContext.h"
namespace Kaidel {

	namespace Utils {
		static VkCommandBufferLevel CommandBufferTypeToVulkanLevel(CommandBufferType type) {
			switch (type)
			{
			case Kaidel::CommandBufferType::Primary:return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			case Kaidel::CommandBufferType::Secondary:return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			}

			KD_CORE_ASSERT(false, "Unknown type");
			return VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
		}
	}


	VulkanCommandBuffer::VulkanCommandBuffer(Ref<CommandPool> commandPool, CommandBufferType type, uint32_t flags)
		:m_CommandPool(commandPool)
	{
		VkCommandBufferAllocateInfo bufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		bufferInfo.commandBufferCount = 1;
		bufferInfo.commandPool = (VkCommandPool)commandPool->GetRendererID();
		bufferInfo.level = Utils::CommandBufferTypeToVulkanLevel(type);
		VK_ASSERT(vkAllocateCommandBuffers(VK_DEVICE.GetDevice(), &bufferInfo, &m_CommandBuffer));
	}
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(VK_DEVICE.GetDevice(), (VkCommandPool)m_CommandPool->GetRendererID(), 1, &m_CommandBuffer);
	}
	void VulkanCommandBuffer::Begin(uint32_t beginFlags)
	{
		VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		info.flags = beginFlags;
		VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffer, &info));
	}

	void VulkanCommandBuffer::End()
	{
		VK_ASSERT(vkEndCommandBuffer(m_CommandBuffer));
	}

}
