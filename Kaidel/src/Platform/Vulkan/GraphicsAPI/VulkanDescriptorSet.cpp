#include "KDpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanGraphicsContext.h"
#include "VulkanShader.h"
#include "VulkanSampler.h"
#include "VulkanUniformBuffer.h"
#include "VulkanStorageBuffer.h"
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
			buffers.reserve(values.size());
			images.reserve(values.size());

			
			const VulkanBackend::BufferInfo* bufferInfo = nullptr;

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
					if (value.ImageValues.Image) {
						info.imageLayout = Utils::ImageLayoutToVulkanImageLayout(value.ImageValues.Layout);
						info.imageView = ((const VulkanBackend::TextureInfo*)value.ImageValues.Image->GetBackendInfo())->View;
					}
					if (value.ImageValues.ImageSampler) {
						info.sampler = ((const VulkanSampler*)value.ImageValues.ImageSampler.Get())->GetSampler();
					}
					images.push_back(info);
					write.pImageInfo = &images.back();
				}
				break;
				case DescriptorType::UniformBuffer:
						bufferInfo = &((const VulkanUniformBuffer*)value.BufferValues.Buffer.Get())->GetBufferInfo();
				case DescriptorType::StorageBuffer:
				{
					if (!bufferInfo) {
						bufferInfo = &((const VulkanStorageBuffer*)value.BufferValues.Buffer.Get())->GetBufferInfo();
					}
					VkDescriptorBufferInfo info = {};
					info.buffer = bufferInfo->Buffer;
					info.offset = 0;
					info.range = VK_WHOLE_SIZE;
					buffers.push_back(info);
					write.pBufferInfo = &buffers.back();
					bufferInfo = nullptr;
				}
				break;
				}
				writes.push_back(write);
			}
		}
	}

	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetLayoutSpecification& layout){
		std::vector<std::pair<VkDescriptorType,VkShaderStageFlags>> types;
		
		for (auto& [type, flags] : layout.Types) {
			types.push_back({ Utils::DescriptorTypeToVulkanDescriptorType(type), Utils::ShaderStagesToVulkanShaderStageFlags(flags) });
		}

		m_Info = VK_BACKEND->CreateDescriptorSet(
			std::initializer_list<std::pair<VkDescriptorType, VkShaderStageFlags>>(types.data(),types.data() + types.size()));
		m_Values.resize(types.size());
		for (uint32_t i = 0; i < types.size(); ++i) {
			m_Values[i].resize(1);
		}
	}
	
	VulkanDescriptorSet::VulkanDescriptorSet(Ref<Shader> shader, uint32_t setIndex) {
		Ref<VulkanShader> vs = shader;
		const auto& set = vs->GetShaderInfo().Reflection.Sets.at(setIndex);

		m_Info = VK_BACKEND->CreateDescriptorSet(vs->GetShaderInfo(), setIndex);
		m_Values.resize(set.Bindings.size());
		for (uint32_t i = 0; i < set.Bindings.size(); ++i) {
			m_Values[i].resize(set.Bindings.at(i).Count);
		}
		m_NamesToBindings = set.NameToBinding;
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VK_CONTEXT.GetBackend()->DestroyDescriptorSet(m_Info);
	}
	DescriptorSet& VulkanDescriptorSet::Update(Ref<Buffer> buffer, uint32_t binding, uint32_t arrayIndex)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.offset = 0;
		bufferInfo.buffer = ((VulkanBackend::BufferInfo*)buffer->GetBackendID())->Buffer;
		bufferInfo.range = VK_WHOLE_SIZE;
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.dstArrayElement = 0;
		write.dstBinding = binding;
		write.dstSet = m_Info.Set;
		write.pBufferInfo = &bufferInfo;
		switch (buffer->GetBufferType())
		{
		case BufferType::UniformBuffer: write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
		case BufferType::StorageBuffer: write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
		}

		VK_BACKEND->UpdateDescriptorSet(m_Info, { write });

		m_Values[binding][arrayIndex].BufferValues.Buffer = buffer;
		return *this;
	}
	DescriptorSet& VulkanDescriptorSet::Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, uint32_t binding, uint32_t arrayIndex)
	{
		VkDescriptorImageInfo imageInfo{};
		if (image) {
			imageInfo.imageLayout = Utils::ImageLayoutToVulkanImageLayout(layout);
			imageInfo.imageView = ((const VulkanBackend::TextureInfo*)image->GetBackendInfo())->View;
		}
		if (sampler) {
			imageInfo.sampler = ((const VulkanSampler*)sampler.Get())->GetSampler();
		}
		VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.descriptorCount = 1;
		write.dstArrayElement = arrayIndex;
		write.dstBinding = binding;
		write.dstSet = m_Info.Set;
		write.pImageInfo = &imageInfo;
		write.descriptorType =
			image && sampler ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER :
			image ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE :
			sampler ? VK_DESCRIPTOR_TYPE_SAMPLER : VK_DESCRIPTOR_TYPE_MAX_ENUM;

		VK_BACKEND->UpdateDescriptorSet(m_Info, { write });

		m_Values[binding][arrayIndex].ImageValues.Image = image;
		m_Values[binding][arrayIndex].ImageValues.ImageSampler = sampler;
		m_Values[binding][arrayIndex].ImageValues.Layout = layout;

		return *this;
	}
}
