#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"	
#include "Kaidel/Renderer/GraphicsAPI/CommandPool.h"

namespace Kaidel {
	class VulkanCommandPool : public CommandPool{
	public:
		VulkanCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		VulkanCommandPool(CommandPoolOperationType opType, CommandPoolFlags flags);
		~VulkanCommandPool();
		VkCommandPool GetCommandPool() const { return m_CommandPool; }

		VkCommandBuffer BeginSingleTimeCommands(VkCommandBufferUsageFlags flags);

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VkFence signalFence);

		void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);

		virtual void Reset(int flags) override;
		virtual RendererID GetRendererID()const override { return (RendererID)m_CommandPool; }
	private:
		VkCommandPool m_CommandPool;
	};

}
