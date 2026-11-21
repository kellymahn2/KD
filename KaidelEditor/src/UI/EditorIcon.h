#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/Sampler.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

#include <unordered_map>
namespace Kaidel {
	class EditorIcon {
	public:
		EditorIcon() = default;

		Ref<Texture2D> GetTexture() const{ return m_Texture; }
		Ref<DescriptorSet> GetDescriptorSet() const { return m_Descriptor; }

		EditorIcon(const Path& path, Ref<Sampler> sampler) {
			m_Texture = TextureLibrary::Load(path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
			DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
			m_Descriptor = DescriptorSet::Create(specs);

			m_Descriptor->Update(m_Texture, sampler, ImageLayout::ShaderReadOnlyOptimal, 0);
		}

	private:
		Ref<Texture2D> m_Texture;
		Ref<DescriptorSet> m_Descriptor;
		static inline std::unordered_map<Path, EditorIcon> s_Icons;
	};

}
