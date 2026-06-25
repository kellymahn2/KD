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
			VK_CONTEXT.SubmitSingle(VK_TRANSFER_Q,
			[&](VkCommandBuffer commandBuffer)
			{
				VkBufferCopy region{};
				region.srcOffset = 0;
				region.dstOffset = 0;
				region.size = size;
				backend->CommandCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, { region });
			});
        }

        backend->DestroyBuffer(stagingBuffer);
    }
    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VK_CONTEXT.GetBackend()->DestroyBuffer(m_Buffer);
    }
}
