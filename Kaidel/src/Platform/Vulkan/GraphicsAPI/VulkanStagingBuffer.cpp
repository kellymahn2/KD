#include "KDpch.h"
#include "VulkanStagingBuffer.h"
#include "VulkanGraphicsContext.h"
namespace Kaidel {
	VulkanStagingBuffer::VulkanStagingBuffer(uint64_t size)
	{
		m_StagingBuffer = VK_BACKEND->CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
		m_MappedBufferBegin = VK_BACKEND->BufferMap(m_StagingBuffer);
		m_MappedBufferCurrent = m_MappedBufferBegin;
		m_MappedBufferEnd = m_MappedBufferBegin + size;
	}
	VulkanStagingBuffer::~VulkanStagingBuffer()
	{
		VK_BACKEND->BufferUnmap(m_StagingBuffer);
		VK_BACKEND->DestroyBuffer(m_StagingBuffer);
	}
	bool VulkanStagingBuffer::HasUnusedSpace(uint64_t dataSize) {
		return m_MappedBufferCurrent + dataSize < m_MappedBufferEnd;
	}

	void VulkanStagingBuffer::AddCopyOperation(
		VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t dataSize, 
		VkAccessFlags access, VkPipelineStageFlags stageFlags)
	{
		KD_CORE_ASSERT(HasUnusedSpace(dataSize));

		// Copy data to the mapped staging buffer
		memcpy(m_MappedBufferCurrent, data, dataSize);

		VK_BACKEND->BufferFlush(m_StagingBuffer, m_MappedBufferCurrent - m_MappedBufferBegin, dataSize);

		// Prepare the buffer copy region
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = static_cast<VkDeviceSize>(m_MappedBufferCurrent - m_MappedBufferBegin);
		copyRegion.dstOffset = 0;  // Adjust if you need to copy to a specific offset in the destination buffer
		copyRegion.size = dataSize;

		// Insert a barrier before the copy to ensure the destination buffer is not being used
		VkBufferMemoryBarrier beforeCopyBarrier = {};
		beforeCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		beforeCopyBarrier.srcAccessMask = access; // Previous access types
		beforeCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Next access type
		beforeCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.buffer = buffer;
		beforeCopyBarrier.offset = 0;
		beforeCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer,
			stageFlags, // Source stages
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Destination stage
			0,
			0, nullptr,
			1, &beforeCopyBarrier,
			0, nullptr
		);

		// Record the buffer copy command
		vkCmdCopyBuffer(
			commandBuffer,
			m_StagingBuffer.Buffer,
			buffer,
			1,
			&copyRegion
		);

		// Insert a barrier after the copy to ensure the copy is completed before further operations
		VkBufferMemoryBarrier afterCopyBarrier = {};
		afterCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		afterCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Previous access type
		afterCopyBarrier.dstAccessMask = access; // Next access types
		afterCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.buffer = buffer;
		afterCopyBarrier.offset = 0;
		afterCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Source stage
			stageFlags, // Destination stages
			0,
			0, nullptr,
			1, &afterCopyBarrier,
			0, nullptr
		);

		// Update the current position in the staging buffer
		m_MappedBufferCurrent += dataSize;
	}

	uint8_t* VulkanStagingBuffer::Reserve(uint64_t dataSize)
	{
		KD_CORE_ASSERT(HasUnusedSpace(dataSize));

		uint8_t* current = m_MappedBufferCurrent;

		m_MappedBufferCurrent += dataSize;

		return current;
	}

	void VulkanStagingBuffer::UploadReserved(VkCommandBuffer commandBuffer, VkBuffer buffer, const uint8_t* reservedStart, uint64_t dataSize, VkAccessFlags access, VkPipelineStageFlags stageFlags)
	{
		VK_BACKEND->BufferFlush(m_StagingBuffer, reservedStart - m_MappedBufferBegin, dataSize);

		// Prepare the buffer copy region
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = static_cast<VkDeviceSize>(reservedStart - m_MappedBufferBegin);
		copyRegion.dstOffset = 0;  // Adjust if you need to copy to a specific offset in the destination buffer
		copyRegion.size = dataSize;

		// Insert a barrier before the copy to ensure the destination buffer is not being used
		VkBufferMemoryBarrier beforeCopyBarrier = {};
		beforeCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		beforeCopyBarrier.srcAccessMask = access; // Previous access types
		beforeCopyBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Next access type
		beforeCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		beforeCopyBarrier.buffer = buffer;
		beforeCopyBarrier.offset = 0;
		beforeCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer,
			stageFlags, // Source stages
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Destination stage
			0,
			0, nullptr,
			1, &beforeCopyBarrier,
			0, nullptr
		);

		// Record the buffer copy command
		vkCmdCopyBuffer(
			commandBuffer,
			m_StagingBuffer.Buffer,
			buffer,
			1,
			&copyRegion
		);

		// Insert a barrier after the copy to ensure the copy is completed before further operations
		VkBufferMemoryBarrier afterCopyBarrier = {};
		afterCopyBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		afterCopyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Previous access type
		afterCopyBarrier.dstAccessMask = access; // Next access types
		afterCopyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		afterCopyBarrier.buffer = buffer;
		afterCopyBarrier.offset = 0;
		afterCopyBarrier.size = dataSize;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, // Source stage
			stageFlags, // Destination stages
			0,
			0, nullptr,
			1, &afterCopyBarrier,
			0, nullptr
		);

	}

	void VulkanStagingBuffer::Reset()
	{
		m_MappedBufferCurrent = m_MappedBufferBegin;
	}

}
