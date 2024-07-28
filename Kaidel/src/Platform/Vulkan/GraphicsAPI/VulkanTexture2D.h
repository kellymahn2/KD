#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"


namespace Kaidel {
	class VulkanTexture2D : public ImmutableTexture2D {
	public:

		VulkanTexture2D(const Texture2DSpecification& spec);

		~VulkanTexture2D();

		Image& GetImage() override;
		const Texture2DSpecification& GetSpecification()const override { return m_Specification; }

		void AddLayer(void* data, uint64_t size) override;

	private:
		Texture2DSpecification m_Specification;
		VkImage m_VulkanImage;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		VkDescriptorSet m_DescriptorSet;

		Image m_Image;

	};
}
