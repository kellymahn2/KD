#include "KDpch.h"
#include "VulkanIndexBuffer.h"
#include "VulkanGraphicsContext.h"
namespace Kaidel {
	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
		m_IndexBuffer = Utils::CreateBuffer(VK_ALLOCATOR.GetAllocator(), count * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* mappedData = nullptr;
		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), m_IndexBuffer.Allocation, &mappedData);
		std::memcpy((char*)mappedData, indices, count * sizeof(uint32_t));
		vmaUnmapMemory(VK_ALLOCATOR.GetAllocator(), m_IndexBuffer.Allocation);
		vmaFlushAllocation(VK_ALLOCATOR.GetAllocator(), m_IndexBuffer.Allocation, 0, VK_WHOLE_SIZE);
	}
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		vmaDestroyBuffer(VK_ALLOCATOR.GetAllocator(), m_IndexBuffer.Buffer, m_IndexBuffer.Allocation);
	}
	void VulkanIndexBuffer::Bind() const
	{
	}
	void VulkanIndexBuffer::Unbind() const
	{
	}
}
