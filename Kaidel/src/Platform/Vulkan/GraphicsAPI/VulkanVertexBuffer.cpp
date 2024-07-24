#include "KDpch.h"
#include "VulkanVertexBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
		for (auto& buffer : m_Buffers) {
			buffer = Utils::CreateBuffer(VK_ALLOCATOR.GetAllocator(), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
	}
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		for (auto& buffer : m_Buffers) {
			vmaDestroyBuffer(VK_ALLOCATOR.GetAllocator(), buffer.Buffer, buffer.Allocation);
		}
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
	{
		auto& buffer = *m_Buffers;
		//Buffer too small
		if (buffer.Size < size) {
			vmaDestroyBuffer(VK_ALLOCATOR.GetAllocator(), buffer.Buffer, buffer.Allocation);
			buffer = Utils::CreateBuffer(VK_ALLOCATOR.GetAllocator(), size * 1.5f,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY);
		}
		VK_CONTEXT.GetBufferStager().Stage(VK_CONTEXT.GetActiveCommandBuffer().Get(), buffer, data, size);
	}
	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		return {};
	}
	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
	}
}
