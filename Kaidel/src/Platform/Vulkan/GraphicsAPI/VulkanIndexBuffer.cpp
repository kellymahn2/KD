#include "KDpch.h"
#include "VulkanIndexBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel{
    VulkanIndexBuffer::VulkanIndexBuffer(const void *indices, uint64_t size, IndexType type)
        :m_IndexType(type)
    {

        const auto& backend = VK_CONTEXT.GetBackend();

        m_Buffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
        uint8_t* mapped = backend->BufferMap(stagingBuffer);
        std::memcpy(mapped, indices, size);
        backend->BufferUnmap(stagingBuffer);

        {
            VkCommandBuffer commandBuffer = backend->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());
            backend->CommandBufferBegin(commandBuffer);

            VkBufferCopy region{};
            region.srcOffset = 0;
            region.dstOffset = 0;
            region.size = size;
            backend->CommandCopyBuffer(commandBuffer,stagingBuffer,m_Buffer,{region});
            
            backend->CommandBufferEnd(commandBuffer);

            backend->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(),{commandBuffer});
            vkQueueWaitIdle(VK_CONTEXT.GetGraphicsQueue());
            backend->DestroyCommandBuffer(commandBuffer,VK_CONTEXT.GetPrimaryCommandPool());
        }

        backend->DestroyBuffer(stagingBuffer);
    }
    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VK_CONTEXT.GetBackend()->DestroyBuffer(m_Buffer);
    }
}
