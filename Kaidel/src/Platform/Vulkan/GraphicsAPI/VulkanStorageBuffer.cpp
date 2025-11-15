#include "KDpch.h"
#include "VulkanStorageBuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanStorageBuffer.h"

namespace Kaidel{
    VulkanStorageBuffer::VulkanStorageBuffer(const void* data, uint64_t size)
    {
		if (size != 0) {
			auto& backend = VK_BACKEND;

			m_Buffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

			if (data)
			{
				VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
				uint8_t* mapped = backend->BufferMap(stagingBuffer);
				std::memcpy(mapped, data, size);
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
    }
    VulkanStorageBuffer::~VulkanStorageBuffer()
    {
		VK_CONTEXT.GetBackend()->DestroyBuffer(m_Buffer);
    }
    void VulkanStorageBuffer::SetData(const void* data, uint64_t size, uint64_t offset)
    {
		auto& buffer = m_Buffer;
		if (buffer.BufferSize < size + offset) {
			VK_CONTEXT.GetBackend()->DestroyBuffer(buffer);
			buffer = VK_CONTEXT.GetBackend()->CreateBuffer(size + offset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}
		VK_CONTEXT.GetBufferStager().StageStorageBuffer(VK_CONTEXT.GetCurrentCommandBuffer(), buffer.Buffer, data, size);
    }

	void* VulkanStorageBuffer::Reserve(uint64_t dataSize)
	{
		auto& buffer = m_Buffer;
		if (buffer.BufferSize < dataSize) {
			VK_CONTEXT.GetBackend()->DestroyBuffer(buffer);

			buffer = VK_CONTEXT.GetBackend()->CreateBuffer(dataSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}

		return VK_CONTEXT.GetBufferStager().Reserve(dataSize);
	}

	void VulkanStorageBuffer::UploadReserve(const void* reservedStart, uint64_t dataSize)
	{
		auto& buffer = m_Buffer;

		VK_CONTEXT.GetBufferStager().
			UploadReserveStorageBuffer(
				VK_CONTEXT.GetCurrentCommandBuffer(),
				buffer.Buffer, (const uint8_t*)reservedStart, dataSize
			);
	}
}
