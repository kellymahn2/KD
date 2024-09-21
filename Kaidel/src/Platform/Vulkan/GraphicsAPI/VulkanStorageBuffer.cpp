#include "KDpch.h"
#include "VulkanStorageBuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanStorageBuffer.h"

namespace Kaidel{
    VulkanStorageBuffer::VulkanStorageBuffer(uint64_t size)
    {
        m_Buffer = VK_CONTEXT.GetBackend()->CreateBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
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
}
