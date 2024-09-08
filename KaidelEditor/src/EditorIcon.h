#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/Sampler.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"


namespace Kaidel {
	class EditorIcon {
	public:
		EditorIcon() = default;
		EditorIcon(const Path& path, Ref<Sampler> sampler) {
			//m_Texture = TextureLibrary::LoadStaticTexture(path);
			//m_Descriptor = DescriptorSet::Create(DescriptorType::CombinedSampler, ShaderStage_FragmentShader);
			//
			//DescriptorSetUpdate update{};
			//update.ArrayIndex = 0;
			//update.Binding = 0;
			//update.Type = DescriptorType::CombinedSampler;
			//update.ImageUpdate.Layout = ImageLayout::ShaderReadOnlyOptimal;
			//update.ImageUpdate.Sampler = sampler->GetRendererID();
			//update.ImageUpdate.ImageView = m_Texture->GetImage()->GetSpecification().ImageView;
			//m_Descriptor->UpdateAll(update);
		}

		Ref<Texture2D> GetTexture() const{ return m_Texture; }
		Ref<DescriptorSet> GetDescriptorSet() const { return m_Descriptor; }

	private:
		Ref<Texture2D> m_Texture;
		Ref<DescriptorSet> m_Descriptor;
	};

}
