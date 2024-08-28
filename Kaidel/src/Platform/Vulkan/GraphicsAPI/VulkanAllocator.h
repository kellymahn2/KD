#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Image.h"
#include <VMA.h>


namespace Kaidel {


	struct ImageAllocateSpecification {
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Depth = 1;
		uint32_t Layers = 1;
		uint32_t Levels = 1;
		Format ImageFormat = Format::RGBA8UN;
		ImageLayout InitialLayout = ImageLayout::None;
		VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;

		VmaMemoryUsage MemoryUsage = VMA_MEMORY_USAGE_AUTO;
		VkImageUsageFlags ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkImageType Type = VK_IMAGE_TYPE_2D;
		VkFlags VulkanFlags = 0;
		uint32_t VmaFlags = 0;
	};


	class VulkanAllocator {
	public:
		VulkanAllocator();
		~VulkanAllocator();

		VulkanBuffer AllocateBuffer(uint64_t size, VmaMemoryUsage memoryUsage, VkBufferUsageFlags usageFlags);

		void DestroyBuffer(VulkanBuffer& buffer);
		void DestroyImage(ImageSpecification& image);
		ImageSpecification AllocateImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t samples,
								uint32_t mipLevels, Format imageFormat, ImageLayout initialLayout, VmaMemoryUsage memoryUsage,
								VkImageUsageFlags usageFlags, VkImageType type, uint32_t flags = 0);

		ImageSpecification AllocateImage(const ImageAllocateSpecification& spec);

		VmaAllocator GetAllocator()const { return m_Allocator; }

	private:
		VmaAllocator m_Allocator;
	};

}

