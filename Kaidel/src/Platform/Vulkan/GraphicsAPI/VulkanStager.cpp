#include "KDpch.h"
#include "VulkanStager.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanBufferStager::VulkanBufferStager(uint64_t eachBufferSize, uint32_t maximumStagingBuffersPerFrame)
		:m_EachBufferSize(eachBufferSize)
	{
		for (auto& block : m_Blocks) {
			block = CreateFrameBlock(eachBufferSize, maximumStagingBuffersPerFrame);
		}
	}

	void VulkanBufferStager::Stage(VulkanCommandBuffer* commandBuffer, const VulkanBuffer& buffer, const void* data, uint64_t size)
	{
		VulkanStagingBuffer* stagingBuffer = m_Blocks->StagingBuffers[m_Blocks->CurrentStagingBuffer];

		if (stagingBuffer->HasUnusedSpace(size)) {
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size);
		}
		else {
			++m_Blocks->CurrentStagingBuffer;
			KD_CORE_ASSERT(m_Blocks->CurrentStagingBuffer < m_Blocks->StagingBuffers.size());
			if (!m_Blocks->StagingBuffers[m_Blocks->CurrentStagingBuffer])
				m_Blocks->StagingBuffers[m_Blocks->CurrentStagingBuffer] = new VulkanStagingBuffer(m_EachBufferSize);
			stagingBuffer = m_Blocks->StagingBuffers[m_Blocks->CurrentStagingBuffer];
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size);
		}
	}
	void VulkanBufferStager::Reset()
	{
		for (auto& stagingBuffer : m_Blocks->StagingBuffers) {
			if (stagingBuffer)
				stagingBuffer->Reset();
		}
	}
	VulkanBufferStager::PerFrameBlock VulkanBufferStager::CreateFrameBlock(uint32_t bufferSize, uint32_t maxStagingBuffersPerFrame)
	{
		PerFrameBlock block{};
		block.CurrentStagingBuffer = 0;
		block.StagingBuffers.resize(maxStagingBuffersPerFrame);
		block.StagingBuffers[0] = new VulkanStagingBuffer(bufferSize);
		return block;
	}
}
