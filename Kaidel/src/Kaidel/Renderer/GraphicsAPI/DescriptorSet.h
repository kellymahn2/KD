#pragma once
#include "Kaidel/Core/Base.h"

#include "UniformBuffer.h"
#include "StorageBuffer.h"
#include "Sampler.h"
#include "GraphicsPipeline.h"
#include "Texture.h"

namespace Kaidel {


	struct DescriptorSetBufferValues {			
		Ref<Kaidel::Buffer> Buffer;

		DescriptorSetBufferValues(Ref<Kaidel::Buffer> buffer = {})
			:Buffer(buffer)
		{}
	};

	struct DescriptorSetImageValues {
		Ref<Sampler> ImageSampler;
		Ref<Texture> Image;
		ImageLayout Layout;

		DescriptorSetImageValues(Ref<Texture> image = {}, ImageLayout layout = {}, Ref<Sampler> sampler = {})
			:Image(image), ImageSampler(sampler), Layout(layout)
		{}
	};

	struct DescriptorValues
	{
		DescriptorType Type;
		DescriptorSetBufferValues BufferValues;
		DescriptorSetImageValues ImageValues;

		DescriptorValues() = default;
		DescriptorValues(Ref<Buffer> buffer)
		//TODO: should actually be taken from the input buffer.
			:BufferValues(buffer)
		{
			switch (buffer->GetBufferType())
			{
			case BufferType::UniformBuffer: Type = DescriptorType::UniformBuffer; break;
			case BufferType::StorageBuffer: Type = DescriptorType::StorageBuffer; break;
			default:
				KD_CORE_ASSERT(false);
			}
		}

		DescriptorValues(Ref<Texture> texture, ImageLayout layout, Ref<Sampler> sampler)
			: Type(texture && sampler ? 
					DescriptorType::SamplerWithTexture : texture ? 
					DescriptorType::Texture : sampler ? 
					DescriptorType::Sampler : DescriptorType::Count) ,
			ImageValues(texture, layout, sampler)
		{}
	};

	struct DescriptorSetSpecification {
		std::vector<DescriptorValues> Values;
		std::vector<ShaderStages> Stages;
		Ref<Shader> Shader;
		uint32_t Set;
	};

	struct DescriptorSetLayoutSpecification {
		std::vector<std::pair<DescriptorType,ShaderStages>> Types;

		DescriptorSetLayoutSpecification(std::initializer_list<std::pair<DescriptorType,ShaderStages>> types)
			:Types(types)
		{}
	};

	class DescriptorSet : public IRCCounter<false> {
	public:
		
		virtual ~DescriptorSet() = default;

		virtual const DescriptorSetSpecification& GetSpecification()const = 0;
		virtual Ref<StorageBuffer> GetStorageBufferAtBinding(uint32_t i)const = 0;
		virtual Ref<UniformBuffer> GetUniformBufferAtBinding(uint32_t i)const = 0;
		virtual Ref<Texture> GetTextureAtBinding(uint32_t i)const = 0;
		virtual Ref<Sampler> GetSamplerAtBinding(uint32_t i)const = 0;
		virtual RendererID GetSetID()const = 0;

		virtual DescriptorSet& Update(Ref<Buffer> buffer, uint32_t binding) = 0;
		virtual DescriptorSet& Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, uint32_t binding) = 0;

		static Ref<DescriptorSet> Create(Ref<Shader> shader, uint32_t setIndex);
		static Ref<DescriptorSet> Create(const DescriptorSetLayoutSpecification& specs);
	};
}
