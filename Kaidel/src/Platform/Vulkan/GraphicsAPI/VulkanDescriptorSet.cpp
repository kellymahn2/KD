#include "KDpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanGraphicsContext.h"
#include "VulkanShader.h"
#include "VulkanSampler.h"
namespace Kaidel {

	namespace Utils {
		static void ValuesToWrites(
			const std::vector<DescriptorValues>& values,
			std::vector<VkWriteDescriptorSet>& writes, 
			std::vector< VkDescriptorImageInfo>& images, std::vector<VkDescriptorBufferInfo>& buffers) 
		{
			writes = {};
			images = {};
			buffers = {};

			for (uint32_t i = 0; i < values.size(); ++i) {
				const auto& value = values[i];
				VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				write.descriptorCount = 1;
				write.descriptorType = DescriptorTypeToVulkanDescriptorType(value.Type);
				write.dstArrayElement = 0;
				write.dstBinding = (uint32_t)i;
				switch (value.Type)
				{
				case DescriptorType::Sampler:
				case DescriptorType::SamplerWithTexture:
				case DescriptorType::Texture:
				{
					VkDescriptorImageInfo info = {};
					info.imageLayout = Utils::ImageLayoutToVulkanImageLayout(value.ImageValues.Layout);
					info.imageView = ((const VulkanBackend::TextureInfo*)value.ImageValues.Image->GetBackendInfo())->View;
					info.sampler = ((const VulkanSampler*)value.ImageValues.ImageSampler.Get())->GetSampler();
					images.push_back(info);
					write.pImageInfo = &images.back();
				}
				break;
				case DescriptorType::UniformBuffer:
				case DescriptorType::StorageBuffer:
				{
					VkDescriptorBufferInfo info = {};
					info.buffer = ((const VulkanBackend::BufferInfo*)value.BufferValues.Buffer->GetRendererID())->Buffer;
					info.offset = 0;
					info.range = VK_WHOLE_SIZE;
					buffers.push_back(info);
					write.pBufferInfo = &buffers.back();
				}
				break;
				}
				writes.push_back(write);
			}
		}
	}

	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetSpecification& specs)
		:m_Specification(specs)
	{

		std::vector<VkWriteDescriptorSet> writes;
		std::vector<VkDescriptorImageInfo> images;
		std::vector<VkDescriptorBufferInfo> buffers;

		Utils::ValuesToWrites(specs.Values, writes, images, buffers);

		if (specs.Shader) {
			const auto& shader = ((VulkanShader*)specs.Shader.Get())->GetShaderInfo();
			
			KD_CORE_ASSERT(specs.Set < shader.DescriptorSetLayouts.size());

			m_Info = VK_CONTEXT.GetBackend()->CreateDescriptorSet(writes, shader, specs.Set);
		}
		else {
			std::vector<VkShaderStageFlags> flags;
			for (auto& flag : specs.Stages) {
				flags.push_back(Utils::ShaderStagesToVulkanShaderStageFlags(flag));
			}
			m_Info = VK_CONTEXT.GetBackend()->CreateDescriptorSet(writes, flags);
		}
	}
	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VK_CONTEXT.GetBackend()->DestroyDescriptorSet(m_Info);
	}
}
