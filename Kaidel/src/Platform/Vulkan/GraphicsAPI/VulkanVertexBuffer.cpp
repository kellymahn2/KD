#include "KDpch.h"
#include "VulkanVertexBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel{
    VulkanVertexBuffer::VulkanVertexBuffer(const void* initData, uint64_t size)
    {
		if (size != 0) {
			auto& backend = VK_BACKEND;

			m_Buffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

			VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
			uint8_t* mapped = backend->BufferMap(stagingBuffer);
			std::memcpy(mapped, initData, size);
			backend->BufferUnmap(stagingBuffer);

			{
				VkCommandBuffer commandBuffer = backend->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());
				backend->CommandBufferBegin(commandBuffer);

				VkBufferCopy region{};
				region.srcOffset = 0;
				region.dstOffset = 0;
				region.size = size;
				backend->CommandCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, { region });

				backend->CommandBufferEnd(commandBuffer);

				backend->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(), { commandBuffer });
				vkQueueWaitIdle(VK_CONTEXT.GetGraphicsQueue());
				backend->DestroyCommandBuffer(commandBuffer, VK_CONTEXT.GetPrimaryCommandPool());
			}

			backend->DestroyBuffer(stagingBuffer);
		}
    }
    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
		if(m_Buffer.Buffer)
			VK_CONTEXT.GetBackend()->DestroyBuffer(m_Buffer);
    }
    void VulkanVertexBuffer::SetData(const void *data, uint64_t size, uint64_t offset)
    {   
        auto& buffer = m_Buffer;
        if(buffer.BufferSize < size + offset){
			if(buffer.Buffer)
				VK_CONTEXT.GetBackend()->DestroyBuffer(buffer);

            buffer = VK_CONTEXT.GetBackend()->CreateBuffer(size + offset, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        }

        VK_CONTEXT.GetBufferStager().StageVertexBuffer(VK_CONTEXT.GetCurrentCommandBuffer(),buffer.Buffer, data, size);
    }
}
