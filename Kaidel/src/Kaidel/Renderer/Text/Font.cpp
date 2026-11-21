#include "KDpch.h"
#include "Font.h"
#include "Kaidel/Renderer/RendererGlobals.h"
#include "MSDFData.h"

namespace Kaidel {

	Font::~Font() {
		delete m_Data;
	}

	void Font::CreateDescriptor()
	{
		DescriptorSetLayoutSpecification specs(
			{
				{DescriptorType::Texture,ShaderStage_FragmentShader},
				{DescriptorType::Sampler,ShaderStage_FragmentShader}
			}
		);

		m_Set = DescriptorSet::Create(specs);

		m_Set->Update(m_AtlasTexture, {}, ImageLayout::ShaderReadOnlyOptimal, 0, 0);
		m_Set->Update({}, RendererGlobals::GetSampler(SamplerFilter::Linear,SamplerMipMapMode::Nearest), {}, 1, 0);
	}
}

