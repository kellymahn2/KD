#include "KDpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanDescriptorSet::VulkanDescriptorSet(Ref<GraphicsPipeline> pipeline, uint32_t setBinding)
	{
		Ref<VulkanGraphicsPipeline> vulkanPipeline = pipeline;
		VkDescriptorSetLayout layout = vulkanPipeline->GetSetLayout(setBinding);

		std::vector<VkDescriptorSetLayout> layouts{ m_Sets.GetResources().size(),layout };

		VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorSetCount = m_Sets.GetResources().size();
		allocateInfo.pSetLayouts = layouts.data();
		allocateInfo.descriptorPool = VK_CONTEXT.GetGlobalDescriptorPool().GetDescriptorPool();

		vkAllocateDescriptorSets(VK_DEVICE.GetDevice(), &allocateInfo, m_Sets.GetResources().data());
	}
	VulkanDescriptorSet::VulkanDescriptorSet(DescriptorType type,ShaderStages flags)
	{
		VkDescriptorSetLayout layout = VK_CONTEXT.GetSingleDescriptorSetLayout(Utils::DescriptorTypeToVulkanDescriptorType(type),
																				Utils::ShaderStagesToVulkanShaderStageFlags(flags));

		std::vector<VkDescriptorSetLayout> layouts{ m_Sets.GetResources().size(),layout };
		VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorSetCount = m_Sets.GetResources().size();
		allocateInfo.pSetLayouts = layouts.data();
		allocateInfo.descriptorPool = VK_CONTEXT.GetGlobalDescriptorPool().GetDescriptorPool();

		vkAllocateDescriptorSets(VK_DEVICE.GetDevice(), &allocateInfo, m_Sets.GetResources().data());
	}
	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		vkFreeDescriptorSets(VK_DEVICE.GetDevice(), VK_CONTEXT.GetGlobalDescriptorPool().GetDescriptorPool(), m_Sets.GetResources().size(), m_Sets.GetResources().data());
	}
	void VulkanDescriptorSet::Update(const DescriptorSetUpdate& update)
	{
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

		write.descriptorCount = 1;
		write.descriptorType = Utils::DescriptorTypeToVulkanDescriptorType(update.Type);
		write.dstArrayElement = update.ArrayIndex;
		write.dstBinding = update.Binding;
		write.dstSet = *m_Sets;

		VkDescriptorImageInfo imageInfo{};
		VkDescriptorBufferInfo bufferInfo{}; 
		switch (update.Type)
		{
		case Kaidel::DescriptorType::Sampler:
		case Kaidel::DescriptorType::CombinedSampler:
		case Kaidel::DescriptorType::Texture:
		{
			
			imageInfo.imageLayout = Utils::ImageLayoutToVulkanImageLayout(update.ImageUpdate.Layout);
			imageInfo.imageView = (VkImageView)update.ImageUpdate.ImageView;
			imageInfo.sampler = (VkSampler)update.ImageUpdate.Sampler;
			write.pImageInfo = &imageInfo;
		}
		break;
		case Kaidel::DescriptorType::ImageBuffer:
		case Kaidel::DescriptorType::UniformBuffer:
		case Kaidel::DescriptorType::StorageBuffer: 
		{
			bufferInfo.buffer = (VkBuffer)update.BufferUpdate.Buffer;
			bufferInfo.offset = update.BufferUpdate.Offset;
			bufferInfo.range = update.BufferUpdate.Size;
			write.pBufferInfo = &bufferInfo;
		}
		break;
		default:
			return;
		}

		vkUpdateDescriptorSets(VK_DEVICE.GetDevice(), 1, &write, 0, nullptr);
	}
	void VulkanDescriptorSet::UpdateAll(const DescriptorSetUpdate& update)
	{
		for (auto& set : m_Sets) {
			VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

			write.descriptorCount = 1;
			write.descriptorType = Utils::DescriptorTypeToVulkanDescriptorType(update.Type);
			write.dstArrayElement = update.ArrayIndex;
			write.dstBinding = update.Binding;
			write.dstSet = set;

			VkDescriptorImageInfo imageInfo{};
			VkDescriptorBufferInfo bufferInfo{};
			switch (update.Type)
			{
			case Kaidel::DescriptorType::Sampler:
			case Kaidel::DescriptorType::CombinedSampler:
			case Kaidel::DescriptorType::Texture:
			{

				imageInfo.imageLayout = Utils::ImageLayoutToVulkanImageLayout(update.ImageUpdate.Layout);
				imageInfo.imageView = (VkImageView)update.ImageUpdate.ImageView;
				imageInfo.sampler = (VkSampler)update.ImageUpdate.Sampler;
				write.pImageInfo = &imageInfo;
			}
			break;
			case Kaidel::DescriptorType::ImageBuffer:
			case Kaidel::DescriptorType::UniformBuffer:
			case Kaidel::DescriptorType::StorageBuffer:
			{
				bufferInfo.buffer = (VkBuffer)update.BufferUpdate.Buffer;
				bufferInfo.offset = update.BufferUpdate.Offset;
				bufferInfo.range = update.BufferUpdate.Size;
				write.pBufferInfo = &bufferInfo;
			}
			break;
			default:
				return;
			}

			vkUpdateDescriptorSets(VK_DEVICE.GetDevice(), 1, &write, 0, nullptr);
		}
		

	}
}
