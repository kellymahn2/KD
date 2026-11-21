#include "KDpch.h"
#include "VulkanUniformBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanUniformBuffer::VulkanUniformBuffer(uint64_t size, const void* initData)
	{
		m_Buffer = VK_CONTEXT.GetBackend()->CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		if (initData)
		{
			auto& backend = VK_BACKEND;

			VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
			uint8_t* mapped = backend->BufferMap(stagingBuffer);
			std::memcpy(mapped, initData, size);
			backend->BufferUnmap(stagingBuffer);

			{
				VK_CONTEXT.SubmitSingle(VK_TRANSFER_Q, 
				[&](VkCommandBuffer commandBuffer)
				{
						VkBufferCopy region{};
						region.srcOffset = 0;
						region.dstOffset = 0;
						region.size = size;
						backend->CommandCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, { region });
				});
			}

			backend->DestroyBuffer(stagingBuffer);
		}
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VK_CONTEXT.GetBackend()->DestroyBuffer(m_Buffer);
	}
	void VulkanUniformBuffer::SetData(const void* data, uint64_t size, uint64_t offset)
	{
		auto& buffer = m_Buffer;
		if (buffer.BufferSize < size + offset) {
			VK_CONTEXT.GetBackend()->DestroyBuffer(buffer);

			buffer = VK_CONTEXT.GetBackend()->CreateBuffer(size + offset, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}

		auto& stager = VK_CONTEXT.GetBufferStager();

		VK_CONTEXT.GetBufferStager().StageUniformBuffer(VK_CONTEXT.GetCurrentCommandBuffer(), buffer.Buffer, data, size);
	}

	uint64_t VulkanUniformBuffer::GetSize() const
	{
		return m_Buffer.BufferSize;
	}

}
