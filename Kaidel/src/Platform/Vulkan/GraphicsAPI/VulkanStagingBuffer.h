#pragma once
#include "VulkanAllocator.h"
#include "Backend.h"

namespace Kaidel {
	struct StagingBufferSyncObjects {

		std::vector<VkSemaphore> WaitSemaphores;
		std::vector<VkSemaphore> SignalSemaphores;
		VkFence SignalFence;


		StagingBufferSyncObjects(const std::vector<VkSemaphore>& waitSemaphores = {},
			const std::vector<VkSemaphore>& signalSemaphores = {}, VkFence signalFence = VK_NULL_HANDLE)
			:WaitSemaphores(waitSemaphores), SignalSemaphores(signalSemaphores), SignalFence(signalFence)
		{}
	};


	class VulkanStagingBuffer {
	public:
		VulkanStagingBuffer(uint64_t size);
		~VulkanStagingBuffer();
		bool HasUnusedSpace(uint64_t dataSize);

		void AddCopyOperation(
			VkCommandBuffer commandBuffer, VkBuffer buffer, const void* data, uint64_t dataSize, VkAccessFlags access, VkPipelineStageFlags stageFlags);

		void Reset();

	private:
		struct CopyContext {
			VkBuffer DstBuffer;
			uint8_t* BufferStart;
			uint8_t* BufferEnd;
		};
	private:

	private:

		VulkanBackend::BufferInfo m_StagingBuffer;

		uint8_t* m_MappedBufferBegin = nullptr;
		uint8_t* m_MappedBufferCurrent = nullptr;
		uint8_t* m_MappedBufferEnd = nullptr;
	};


}
