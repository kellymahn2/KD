#include "KDpch.h"
#include "VulkanUniformBuffer.h"
#include "VulkanGraphicsContext.h"

#include "VulkanMemory.h"


namespace Kaidel {
	namespace Vulkan {
		VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
			:m_Binding(binding)
		{
			Utils::BufferSpecification spec{};
			spec.BufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			spec.LogicalDevice = VK_DEVICE;
			spec.PhysicalDevice = VK_PHYSICAL_DEVICE;
			spec.QueueFamilies = VK_UNIQUE_INDICES;
			spec.Size = size;
			spec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			for (auto& buffer : m_Buffer) {
				Utils::BufferCreateResult result = Utils::CreateBuffer(spec);
				buffer = VulkanBuffer(result.Buffer, result.AllocatedMemory, size);
			}
		}
		VulkanUniformBuffer::~VulkanUniformBuffer()
		{
			for (auto& buffer : m_Buffer) {
				Utils::DeleteBuffer(VK_DEVICE, buffer.Buffer, buffer.DeviceMemory);
			}
		}
		void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
		{
			uint32_t mappedSize = size;
			
			if (size > m_Buffer->Size)
				mappedSize = m_Buffer->Size;

			m_Buffer->Map(VK_DEVICE, mappedSize, offset, 0);
			memcpy(m_Buffer->MappedMemory, data, mappedSize);
			m_Buffer->Unmap(VK_DEVICE);
		}
		void VulkanUniformBuffer::Bind(uint32_t binding)
		{
		}
		void VulkanUniformBuffer::Bind()
		{
		}
		void VulkanUniformBuffer::UnBind()
		{
		}
	}
}
