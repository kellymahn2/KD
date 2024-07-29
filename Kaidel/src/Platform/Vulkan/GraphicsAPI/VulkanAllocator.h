#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Image.h"
#include <VMA.h>


namespace Kaidel {
	class VulkanAllocator {
	public:
		VulkanAllocator();
		~VulkanAllocator();

		VulkanBuffer AllocateBuffer(uint64_t size, VmaMemoryUsage memoryUsage, VkBufferUsageFlags usageFlags);

		void DestroyBuffer(VulkanBuffer& buffer);
		void DestroyImage(Image& image);
		Image AllocateImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t samples,
								uint32_t mipLevels, Format imageFormat, ImageLayout initialLayout, VmaMemoryUsage memoryUsage,
								VkImageUsageFlags usageFlags, VkImageType type);

		VmaAllocator GetAllocator()const { return m_Allocator; }

	private:
		VmaAllocator m_Allocator;
	};

}

