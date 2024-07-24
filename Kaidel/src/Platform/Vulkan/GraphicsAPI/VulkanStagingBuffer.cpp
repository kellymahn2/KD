#include "KDpch.h"
#include "VulkanStagingBuffer.h"
#include "VulkanGraphicsContext.h"
namespace Kaidel {
	VulkanStagingBuffer::VulkanStagingBuffer(uint64_t size)
	{
		m_StagingBuffer = VK_ALLOCATOR.AllocateBuffer(size, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), m_StagingBuffer.Allocation, (void**)&m_MappedBufferBegin);
		m_MappedBufferCurrent = m_MappedBufferBegin;
		m_MappedBufferEnd = m_MappedBufferBegin + size;
	}
	bool VulkanStagingBuffer::HasUnusedSpace(uint64_t dataSize) {
		return m_MappedBufferCurrent + dataSize < m_MappedBufferEnd;
	}
	void VulkanStagingBuffer::AddCopyOperation(VulkanCommandBuffer* commandBuffer, const VulkanBuffer& buffer, const void* data, uint64_t dataSize)
	{
		KD_CORE_ASSERT(HasUnusedSpace(dataSize));

		// Copy data to the mapped staging buffer
		memcpy(m_MappedBufferCurrent, data, dataSize);

		// Prepare the buffer copy region
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = static_cast<VkDeviceSize>(m_MappedBufferCurrent - m_MappedBufferBegin);
		copyRegion.dstOffset = 0;  // Adjust if you need to copy to a specific offset in the destination buffer
		copyRegion.size = dataSize;

		// Insert a barrier before the copy to ensure the destination buffer is not being used
		VkBufferMemoryBarrier beforeCopyBarrier = {};
		beforeCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		beforeCopyBarrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT; // Previous access types
		beforeCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Next access type
		beforeCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.buffer = buffer.Buffer;
		beforeCopyBarrier.offset = 0;
		beforeCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer->GetCommandBuffer(),
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Source stages
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Destination stage
			0,
			0, nullptr,
			1, &beforeCopyBarrier,
			0, nullptr
		);

		// Record the buffer copy command
		vkCmdCopyBuffer(
			commandBuffer->GetCommandBuffer(),
			m_StagingBuffer.Buffer,
			buffer.Buffer,
			1,
			&copyRegion
		);

		// Insert a barrier after the copy to ensure the copy is completed before further operations
		VkBufferMemoryBarrier afterCopyBarrier = {};
		afterCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		afterCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Previous access type
		afterCopyBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT; // Next access types
		afterCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.buffer = buffer.Buffer;
		afterCopyBarrier.offset = 0;
		afterCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer->GetCommandBuffer(),
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Source stage
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Destination stages
			0,
			0, nullptr,
			1, &afterCopyBarrier,
			0, nullptr
		);

		// Update the current position in the staging buffer
		m_MappedBufferCurrent += dataSize;
	}
	void VulkanStagingBuffer::Reset()
	{
		m_MappedBufferCurrent = m_MappedBufferBegin;
	}

}
