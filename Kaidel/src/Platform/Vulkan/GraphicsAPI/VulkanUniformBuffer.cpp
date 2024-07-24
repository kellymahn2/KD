#include "KDpch.h"
#include "VulkanUniformBuffer.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		:m_Size(size),m_Binding(binding)
	{
		for (auto& buffer : m_Buffer) {
			buffer = VK_ALLOCATOR.AllocateBuffer(size, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}
		CreateDescriptorSets();
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		for (auto& buffer : m_Buffer) {
			VK_ALLOCATOR.DestroyBuffer(buffer);
		}
	}
	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		void* mappedData = nullptr;
		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation, &mappedData);
		std::memcpy((char*)mappedData + offset, data, size);
		vmaUnmapMemory(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation);
		vmaFlushAllocation(VK_ALLOCATOR.GetAllocator(), m_Buffer->Allocation, offset, VK_WHOLE_SIZE);
	}
	void VulkanUniformBuffer::CreateDescriptorSets()
	{
		VkDescriptorSetLayout setLayout = VK_CONTEXT.GetUniformBufferDescriptorSetLayouts()[m_Binding];
		std::vector<VkDescriptorSetLayout> layouts(m_Sets.GetResources().size(), setLayout);
		VkDescriptorSetAllocateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		setInfo.descriptorPool = VK_CONTEXT.GetUniformBufferDescriptorPool().GetDescriptorPool();
		setInfo.descriptorSetCount = (uint32_t)m_Sets.GetResources().size();
		setInfo.pSetLayouts = layouts.data();
		vkAllocateDescriptorSets(VK_DEVICE.GetDevice(), &setInfo, m_Sets.GetResources().data());

		std::vector<VkDescriptorBufferInfo> bufferInfos{};
		std::vector<VkWriteDescriptorSet> writeInfos{};

		for (auto& buffer : m_Buffer) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;
			bufferInfo.buffer = buffer.Buffer;
			bufferInfos.push_back(bufferInfo);
		}

		for (auto& set : m_Sets) {
			VkWriteDescriptorSet writeInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			writeInfo.descriptorCount = 1;
			writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeInfo.dstArrayElement = 0;
			writeInfo.dstBinding = m_Binding;
			writeInfo.dstSet = set;
			writeInfo.pBufferInfo = &bufferInfos[writeInfos.size()];
			writeInfos.push_back(writeInfo);
		}

		vkUpdateDescriptorSets(VK_DEVICE.GetDevice(), (uint32_t)writeInfos.size(), writeInfos.data(), 0, nullptr);
	}
}
