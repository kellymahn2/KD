#include "KDpch.h"
#include "VulkanUniformBuffer.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		:m_Size(size),m_Binding(binding)
	{
		for (auto& buffer : m_Buffer) {
			buffer = VK_ALLOCATOR.AllocateBuffer(size, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		for (auto& buffer : m_Buffer) {
			VK_ALLOCATOR.DestroyBuffer(buffer);
		}
	}
	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		void* mappedData = nullptr;
		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation, &mappedData);
		std::memcpy((char*)mappedData + offset, data, size);
		vmaUnmapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation);
		vmaFlushAllocation(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation, offset, VK_WHOLE_SIZE);
	}
}
