#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"


namespace Kaidel {

	class VulkanTexture2D : public Texture2D {
	public:
		VulkanTexture2D(const Texture2DSpecification& spec);
		~VulkanTexture2D();

		Ref<Image> GetImage() const override { return m_Image; }

		const Texture2DSpecification& GetSpecification() const override { return m_Specification; }

		void* Map(uint32_t mipMap) const override;
		void Unmap() const override;

	private:
		Texture2DSpecification m_Specification;
		Ref<Image> m_Image;

		VmaAllocationInfo m_AllocationInfo;
	};


	class VulkanTextureLayered2D : public TextureLayered2D {
	public:
		VulkanTextureLayered2D(const TextureLayered2DSpecification& spec);
		~VulkanTextureLayered2D();

		const TextureLayered2DSpecification& GetSpecification() const override { return m_Specification; }
		const TextureLayered2DLayerSpecification& GetLayerSpecification(uint32_t layer) const override { return m_LayerSpecifications[layer]; }
		
		Ref<Image> GetImage() const override { return m_Image; }
		

		void* Map(uint32_t mipMap, uint32_t layer) const override;
		void Unmap() const override;
		uint32_t Push(const TextureLayered2DLayerSpecification& layerSpec) override;
	private:
		void CopyData(void* data, uint32_t width, uint32_t height, uint64_t size, uint32_t layer);
		void Copy(VkCommandBuffer cmd, Ref<TransferBuffer> src, const BufferToTextureCopyRegion& region);
		void Reallocate();
		void ReallocateAndCopy(void* data, uint32_t width, uint32_t height, uint64_t size, uint32_t layer);
		void DeleteImage();
	private:
		TextureLayered2DSpecification m_Specification;
		Ref<Image> m_Image;
		VmaAllocationInfo m_AllocationInfo;
		std::vector<TextureLayered2DLayerSpecification> m_LayerSpecifications;
	};



}
