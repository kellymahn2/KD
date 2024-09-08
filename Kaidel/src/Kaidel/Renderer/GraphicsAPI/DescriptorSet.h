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
	};

	struct DescriptorSetImageValues {
		Ref<Sampler> ImageSampler;
		Ref<Texture> Image;
		ImageLayout Layout;
	};

	struct DescriptorValues
	{
		DescriptorType Type;
		DescriptorSetBufferValues BufferValues;
		DescriptorSetImageValues ImageValues;
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

		static Ref<DescriptorSet> Create(const DescriptorSetSpecification& specs);
	};
}
