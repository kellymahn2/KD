#include "KDpch.h"
#include "VulkanTransferBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanTransferBuffer::VulkanTransferBuffer(uint64_t size, const void* initData, uint64_t initDataSize)
	{
		if (initDataSize == -1)
			initDataSize = size;

		m_Buffer = VK_ALLOCATOR.AllocateBuffer(size, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		SetDataFromAllocation(initData, initDataSize, 0);
	}
	VulkanTransferBuffer::~VulkanTransferBuffer()
	{
		VK_ALLOCATOR.DestroyBuffer(m_Buffer);
	}
	void* VulkanTransferBuffer::Map()
	{
		void* data = nullptr;
		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer.Allocation, &data);
		return data;
	}
	void VulkanTransferBuffer::Unmap()
	{
		vmaUnmapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer.Allocation);
	}
	void VulkanTransferBuffer::SetDataFromAllocation(const void* allocation, uint64_t size, uint64_t offset)
	{
		vmaCopyMemoryToAllocation(VK_ALLOCATOR.GetAllocator(), allocation, m_Buffer.Allocation, offset, size);
	}
	void VulkanTransferBuffer::GetData(void* out, uint64_t size, uint64_t offset)
	{
		vmaCopyAllocationToMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer.Allocation, offset, out, size);
	}
}
