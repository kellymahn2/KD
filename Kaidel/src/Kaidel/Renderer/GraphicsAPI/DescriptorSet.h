#pragma once
#include "Kaidel/Core/Base.h"

#include "UniformBuffer.h"
#include "StorageBuffer.h"
#include "Sampler.h"
#include "GraphicsPipeline.h"
#include "Texture.h"

namespace Kaidel {


	struct DescriptorSetBufferValues {			
		Ref<ShaderInputBuffer> Buffer;

		DescriptorSetBufferValues(Ref<ShaderInputBuffer> buffer = {})
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
		DescriptorValues(Ref<ShaderInputBuffer> buffer)
		//TODO: should actually be taken from the input buffer.
			:Type(DescriptorType::UniformBuffer),BufferValues(buffer)
		{}

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

	class DescriptorSet : public IRCCounter<false> {
	public:
		
		virtual const DescriptorSetSpecification& GetSpecification()const = 0;
		virtual ~DescriptorSet() = default;

		virtual RendererID GetSetID()const = 0;

		static Ref<DescriptorSet> Create(const DescriptorSetSpecification& specs);
	};
}
