#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/CommandBuffer.h"

namespace Kaidel {

	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(Ref<CommandPool> commandPool, CommandBufferType type, uint32_t flags);
		~VulkanCommandBuffer();


		virtual RendererID GetCommandBufferID()const override { return (RendererID)m_CommandBuffer; }

		VkCommandBuffer GetCommandBuffer()const { return m_CommandBuffer; }

		void Begin(uint32_t beginFlags);
		void End();

	private:
		VkCommandBuffer m_CommandBuffer;
		Ref<CommandPool> m_CommandPool;
	};
}
