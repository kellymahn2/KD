#include "KDpch.h"
#include "EnvironmentMap.h"

namespace Kaidel {


	EnvironmentMap::EnvironmentMap(const EnvironmentMapSpecification& specs)
		: m_Specification(specs)
	{
		ShaderStages stages =
			ShaderStage_FragmentShader;

		DescriptorSetLayoutSpecification layoutSpecs(
			{
				{DescriptorType::SamplerWithTexture, stages},
				{DescriptorType::SamplerWithTexture, stages},
				{DescriptorType::SamplerWithTexture, stages},
				{DescriptorType::SamplerWithTexture, stages}
			}
		);

		m_Set = DescriptorSet::Create(layoutSpecs);

		static Ref<Sampler> clampSampler;
		if (!clampSampler)
		{
			SamplerState state;
			state.MinFilter = SamplerFilter::Linear;
			state.MagFilter = SamplerFilter::Linear;
			state.MipFilter = SamplerMipMapMode::Linear;
			state.AddressModeU = SamplerAddressMode::ClampToEdge;
			state.AddressModeV = SamplerAddressMode::ClampToEdge;
			state.AddressModeW = SamplerAddressMode::Repeat;
			state.MaxLod = 1000.0f;

			clampSampler = Sampler::Create(state);
		}

		static Ref<Sampler> sampler;

		if (!sampler)
		{
			SamplerState state;

			state.MinFilter = SamplerFilter::Linear;
			state.MagFilter = SamplerFilter::Linear;
			state.MipFilter = SamplerMipMapMode::Linear;
			state.MaxLod = 1000.0f;

			sampler = Sampler::Create(state);
		}

		m_Set->
			Update(m_Specification.Environment, sampler, ImageLayout::ShaderReadOnlyOptimal, 0)
			.Update(m_Specification.Irradiance, sampler, ImageLayout::ShaderReadOnlyOptimal, 1)
			.Update(m_Specification.SpecularLUT, clampSampler, ImageLayout::ShaderReadOnlyOptimal, 2)
			.Update(m_Specification.Specular, sampler, ImageLayout::ShaderReadOnlyOptimal, 3);
	}

}
