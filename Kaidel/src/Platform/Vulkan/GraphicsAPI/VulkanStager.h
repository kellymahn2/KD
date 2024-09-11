#pragma once

#include "VulkanStagingBuffer.h"

#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"

namespace Kaidel {
	class VulkanBufferStager {
	public:
		VulkanBufferStager(uint64_t eachBufferSize, uint32_t maximumStagingBuffersPerFrame);
		void StageVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size);
		void StageUniformBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size);
		void StageStorageBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t size);

		void Reset();

	private:
		struct PerFrameBlock {
			std::vector<Scope<VulkanStagingBuffer>> StagingBuffers;
			uint32_t CurrentStagingBuffer = 0;
		};

	private:
		PerFrameBlock CreateFrameBlock(uint32_t bufferSize, uint32_t maxStagingBuffersPerFrame);
	private:
		std::vector<PerFrameBlock> m_Blocks;
		uint64_t m_EachBufferSize;
	};

}
