#include "KDpch.h"
#include "VulkanStager.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanBufferStager::VulkanBufferStager(uint64_t eachBufferSize, uint32_t maximumStagingBuffersPerFrame)
		:m_EachBufferSize(eachBufferSize),m_Blocks(VK_CONTEXT.GetMaxFramesInFlightCount())
	{
		for (auto& block : m_Blocks) {
			block = CreateFrameBlock((uint32_t)eachBufferSize, maximumStagingBuffersPerFrame);
		}
	}

	void VulkanBufferStager::StageVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size)
	{
		auto& block = m_Blocks[VK_CONTEXT.GetCurrentFrameNumber()];
		VulkanStagingBuffer* stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();

		if (stagingBuffer->HasUnusedSpace(size)) {
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
		else {
			++block.CurrentStagingBuffer;
			KD_CORE_ASSERT(block.CurrentStagingBuffer < block.StagingBuffers.size());
			if (!block.StagingBuffers[block.CurrentStagingBuffer])
				block.StagingBuffers[block.CurrentStagingBuffer] = CreateScope<VulkanStagingBuffer>(m_EachBufferSize);
			stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
	}
	void VulkanBufferStager::StageUniformBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size)
	{
		auto& block = m_Blocks[VK_CONTEXT.GetCurrentFrameNumber()];
		VulkanStagingBuffer* stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();

		const VkPipelineStageFlags pipelineFlags =
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		if (stagingBuffer->HasUnusedSpace(size)) {
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, VK_ACCESS_UNIFORM_READ_BIT, pipelineFlags);
		}
		else {
			++block.CurrentStagingBuffer;
			KD_CORE_ASSERT(block.CurrentStagingBuffer < block.StagingBuffers.size());
			if (!block.StagingBuffers[block.CurrentStagingBuffer])
				block.StagingBuffers[block.CurrentStagingBuffer] = CreateScope<VulkanStagingBuffer>(m_EachBufferSize);
			stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, VK_ACCESS_UNIFORM_READ_BIT, pipelineFlags);
		}
	}
	void VulkanBufferStager::StageStorageBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size)
	{
		auto& block = m_Blocks[VK_CONTEXT.GetCurrentFrameNumber()];
		VulkanStagingBuffer* stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();

		const VkAccessFlags accessFlags =
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		const VkPipelineStageFlags pipelineFlags =
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		if (stagingBuffer->HasUnusedSpace(size)) {
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, accessFlags, pipelineFlags);
		}
		else {
			++block.CurrentStagingBuffer;
			KD_CORE_ASSERT(block.CurrentStagingBuffer < block.StagingBuffers.size());
			if (!block.StagingBuffers[block.CurrentStagingBuffer])
				block.StagingBuffers[block.CurrentStagingBuffer] = CreateScope<VulkanStagingBuffer>(m_EachBufferSize);
			stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();
			stagingBuffer->AddCopyOperation(commandBuffer, buffer, data, size, accessFlags, pipelineFlags);
		}
	}


	uint8_t* VulkanBufferStager::Reserve(uint64_t dataSize)
	{
		auto& block = m_Blocks[VK_CONTEXT.GetCurrentFrameNumber()];
		VulkanStagingBuffer* stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();

		if (stagingBuffer->HasUnusedSpace(dataSize)) {
			return stagingBuffer->Reserve(dataSize);
		}
		else {
			++block.CurrentStagingBuffer;
			KD_CORE_ASSERT(block.CurrentStagingBuffer < block.StagingBuffers.size());
			if (!block.StagingBuffers[block.CurrentStagingBuffer])
				block.StagingBuffers[block.CurrentStagingBuffer] = CreateScope<VulkanStagingBuffer>(m_EachBufferSize);
			stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();
			return stagingBuffer->Reserve(dataSize);
		}
	}

	void VulkanBufferStager::UploadReserveStorageBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const uint8_t* reservedStart, uint64_t size)
	{
		auto& block = m_Blocks[VK_CONTEXT.GetCurrentFrameNumber()];
		VulkanStagingBuffer* stagingBuffer = block.StagingBuffers[block.CurrentStagingBuffer].get();

		const VkAccessFlags accessFlags =
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

		const VkPipelineStageFlags pipelineFlags =
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
			VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
			VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		stagingBuffer->UploadReserved(commandBuffer, buffer, reservedStart, size, accessFlags, pipelineFlags);
	}

	void VulkanBufferStager::Reset()
	{
		for (auto& stagingBuffer : m_Blocks[VK_CONTEXT.GetCurrentFrameIndex()].StagingBuffers) {
			if (stagingBuffer)
				stagingBuffer->Reset();
		}

		m_Blocks[VK_CONTEXT.GetCurrentFrameIndex()].CurrentStagingBuffer = 0;
	}
	VulkanBufferStager::PerFrameBlock VulkanBufferStager::CreateFrameBlock(uint32_t bufferSize, uint32_t maxStagingBuffersPerFrame)
	{
		PerFrameBlock block{};
		block.CurrentStagingBuffer = 0;
		block.StagingBuffers.resize(maxStagingBuffersPerFrame);
		block.StagingBuffers[0] = CreateScope<VulkanStagingBuffer>(bufferSize);
		return block;
	}
}
