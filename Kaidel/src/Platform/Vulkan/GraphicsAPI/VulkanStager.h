#pragma once

#include "VulkanStagingBuffer.h"

#include "PerFrameResource.h"

namespace Kaidel {
	class VulkanBufferStager {
	public:
		VulkanBufferStager(uint64_t eachBufferSize, uint32_t maximumStagingBuffersPerFrame);
		void Stage(VulkanCommandBuffer* commandBuffer, const VulkanBuffer& buffer, const void* data, uint64_t size);

		void Reset();


	private:
		struct PerFrameBlock {
			std::vector<Scope<VulkanStagingBuffer>> StagingBuffers;
			uint32_t CurrentStagingBuffer = 0;
		};

	private:
		PerFrameBlock CreateFrameBlock(uint32_t bufferSize, uint32_t maxStagingBuffersPerFrame);
	private:

		PerFrameResource<PerFrameBlock> m_Blocks;
		uint64_t m_EachBufferSize;


	};

}
