#include "KDpch.h"
#include "VulkanAllocator.h"

#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanAllocator::VulkanAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorInfo.instance = VK_INSTANCE.GetInstance();
		allocatorInfo.physicalDevice = VK_PHYSICAL_DEVICE.GetDevice();
		allocatorInfo.device = VK_DEVICE.GetDevice();

		VK_ASSERT(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
	}
	VulkanAllocator::~VulkanAllocator()
	{
		vmaDestroyAllocator(m_Allocator);
	}
	VulkanBuffer VulkanAllocator::AllocateBuffer(uint64_t size, VmaMemoryUsage memoryUsage, VkBufferUsageFlags usageFlags)
	{
		return Utils::CreateBuffer(m_Allocator, size, usageFlags, memoryUsage);
	}
	void VulkanAllocator::DestroyBuffer(VulkanBuffer& buffer)
	{
		vmaDestroyBuffer(m_Allocator, buffer.Buffer, buffer.Allocation);
	}
	void VulkanAllocator::DestroyImage(Image& image)
	{
		vmaDestroyImage(m_Allocator, (VkImage)image._InternalImageID, (VmaAllocation)image._DeviceMemory);
	}
	Image VulkanAllocator::AllocateImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t samples,
											uint32_t mipLevels, Format imageFormat, ImageLayout initialLayout, VmaMemoryUsage memoryUsage,
											VkImageUsageFlags usageFlags, VkImageType type)
	{
		Image result{};
		result.Width = width;
		result.Height = height;
		result.Depth = depth;
		result.Layout = initialLayout;
		result.IntendedLayout = initialLayout;
		result.ImageFormat = imageFormat;
		result.Layers = layers;
		result.Levels = mipLevels;

		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.arrayLayers = depth;
		imageInfo.extent = { width,height,depth };
		imageInfo.imageType = type;
		imageInfo.format = Utils::FormatToVulkanFormat(imageFormat);
		imageInfo.initialLayout = Utils::ImageLayoutToVulkanImageLayout(initialLayout);
		imageInfo.mipLevels = mipLevels;
		imageInfo.samples = VkSampleCountFlagBits(samples);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.usage = usageFlags;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;

		VK_ASSERT(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, (VkImage*)&result._InternalImageID, (VmaAllocation*)&result._DeviceMemory, nullptr));

		return result;
	}
}
