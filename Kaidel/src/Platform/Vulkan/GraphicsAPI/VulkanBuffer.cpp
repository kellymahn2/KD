#include "KDpch.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanMemory.h"

#include "VulkanCommandBuffer.h"

namespace Kaidel {




	namespace Vulkan {
		
		#pragma region VertexBuffer
		VulkanVertexBuffer::VulkanVertexBuffer(const VertexBufferSpecification& specification)
			:m_Specification(specification)
		{

			m_StagingBuffer = VK_CONTEXT.GetGlobalStagingBuffer();

			if (specification.Size != 0) {

				ReadyStagingBuffer(specification.Data, specification.Size);

				auto commandPool = VK_CONTEXT.GetTransferCommandPool();

				VulkanCommandBuffer commandBuffer = VulkanCommandBuffer(commandPool);

				commandBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				for (auto& buffer : m_Buffers) {

					CreateBuffer(buffer, m_Specification.Size);

					VkBufferCopy region{};
					region.size = m_Specification.Size;
					region.srcOffset = 0;
					region.dstOffset = 0;
					vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), m_StagingBuffer.Buffer, buffer.Buffer, 1, &region);
				}

				commandBuffer.EndRecording();

				VkQueue transferQueue = VK_DEVICE_QUEUE("TransferQueue").Queue;

				commandBuffer.Submit(CommandBufferSubmitSpecification(transferQueue));
				VK_ASSERT(vkQueueWaitIdle(transferQueue));
			}
			

			

		}

		VulkanVertexBuffer::~VulkanVertexBuffer()
		{
			for (auto& buffer : m_Buffers) {
				DeleteBuffer(buffer);
			}
		}

		void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
		{
			ReadyStagingBuffer(data, size);

			//Can't fit data into actual buffer
			if (size > m_Buffers->Size) {
				DeleteBuffer(*m_Buffers);
				CreateBuffer(*m_Buffers, size);
			}

			auto commandPool = VK_CONTEXT.GetTransferCommandPool();

			VulkanCommandBuffer commandBuffer = VulkanCommandBuffer(commandPool);

			commandBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			VkBufferCopy region{};
			region.size = size;
			region.srcOffset = 0;
			region.dstOffset = 0;
			vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), m_StagingBuffer.Buffer, m_Buffers->Buffer, 1, &region);

			commandBuffer.EndRecording();

			VkQueue transferQueue = VK_DEVICE_QUEUE("TransferQueue").Queue;

			commandBuffer.Submit(CommandBufferSubmitSpecification(transferQueue));
			VK_ASSERT(vkQueueWaitIdle(transferQueue));

		}


		void VulkanVertexBuffer::ReadyStagingBuffer(const void* data, uint32_t size)
		{
			if (data) {
				KD_CORE_ASSERT(size < m_StagingBuffer.Size);
				memcpy(m_StagingBuffer.MappedMemory, data, size);
			}
		}

		void VulkanVertexBuffer::CreateBuffer(VulkanBuffer& buffer, uint32_t size) const
		{
			Utils::BufferSpecification bufferSpec{};
			bufferSpec.BufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferSpec.LogicalDevice = VK_DEVICE;
			bufferSpec.PhysicalDevice = VK_PHYSICAL_DEVICE;
			bufferSpec.Size = size;
			bufferSpec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			Utils::BufferCreateResult createResult = Utils::CreateBuffer(bufferSpec);

			buffer = VulkanBuffer(createResult.Buffer, createResult.AllocatedMemory, size);
		}

		void VulkanVertexBuffer::DeleteBuffer(VulkanBuffer& buffer) const
		{
			Utils::DeleteBuffer(VK_DEVICE, buffer.Buffer, buffer.DeviceMemory);
		}


		#pragma endregion
		
		#pragma region IndexBuffer 
		VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
			:m_Count(count)
		{
			const VulkanBuffer& stagingBuffer = VK_CONTEXT.GetGlobalStagingBuffer();

			memcpy(stagingBuffer.MappedMemory, indices, count * sizeof(uint32_t));

			uint32_t size = count * sizeof(uint32_t);

			Utils::BufferSpecification bufferSpec{};
			bufferSpec.BufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferSpec.Size = size;
			bufferSpec.LogicalDevice = VK_DEVICE;
			bufferSpec.PhysicalDevice = VK_PHYSICAL_DEVICE;
			bufferSpec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			
			Utils::BufferCreateResult buffer = Utils::CreateBuffer(bufferSpec);

			m_Buffer = buffer.Buffer;
			m_Memory = buffer.AllocatedMemory;

			auto commandPool = VK_CONTEXT.GetTransferCommandPool();

			VulkanCommandBuffer commandBuffer = VulkanCommandBuffer(commandPool);

			commandBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			VkBufferCopy region{};
			region.size = size;
			region.srcOffset = 0;
			region.dstOffset = 0;

			vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), stagingBuffer.Buffer, m_Buffer, 1, &region);

			commandBuffer.EndRecording();

			VkQueue transferQueue = VK_DEVICE_QUEUE("TransferQueue").Queue;
			commandBuffer.Submit(CommandBufferSubmitSpecification(transferQueue));
			VK_ASSERT(vkQueueWaitIdle(transferQueue));
		}
		VulkanIndexBuffer::~VulkanIndexBuffer()
		{
			vkDestroyBuffer(VK_DEVICE, m_Buffer, VK_ALLOCATOR_PTR);
			vkFreeMemory(VK_DEVICE, m_Memory, VK_ALLOCATOR_PTR);
		}
		void VulkanIndexBuffer::Bind() const
		{
			vkCmdBindIndexBuffer(VK_CURRENT_IMAGE.CommandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		void VulkanIndexBuffer::Unbind() const
		{
		}
		#pragma endregion
		
	}
}
