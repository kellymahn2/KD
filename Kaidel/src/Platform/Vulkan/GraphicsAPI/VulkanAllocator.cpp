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
		return {};
		//return Utils::CreateBuffer(m_Allocator, size, usageFlags, memoryUsage);
	}
	void VulkanAllocator::DestroyBuffer(VulkanBuffer& buffer)
	{
		//vmaDestroyBuffer(m_Allocator, buffer.Buffer, buffer.Allocation);
	}
	//void VulkanAllocator::DestroyImage(ImageSpecification& image)
	//{
	//	//vmaDestroyImage(m_Allocator, (VkImage)image._InternalImageID, (VmaAllocation)image._DeviceMemory);
	//}
	//ImageSpecification VulkanAllocator::AllocateImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers, uint32_t samples,
	//										uint32_t mipLevels, Format imageFormat, ImageLayout initialLayout, VmaMemoryUsage memoryUsage,
	//										VkImageUsageFlags usageFlags, VkImageType type,uint32_t flags)
	//{
	//	return {};
	//	/*ImageSpecification result{};
	//	result.Width = width;
	//	result.Height = height;
	//	result.Depth = depth;
	//	result.Layout = initialLayout;
	//	result.IntendedLayout = initialLayout;
	//	result.ImageFormat = imageFormat;
	//	result.Layers = layers;
	//	result.Levels = mipLevels;

	//	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	//	imageInfo.arrayLayers = depth;
	//	imageInfo.extent = { width,height,depth };
	//	imageInfo.imageType = type;
	//	imageInfo.format = Utils::FormatToVulkanFormat(imageFormat);
	//	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	imageInfo.mipLevels = mipLevels;
	//	imageInfo.samples = VkSampleCountFlagBits(samples);
	//	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	//	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//	imageInfo.usage = usageFlags;

	//	VmaAllocationCreateInfo allocInfo{};
	//	allocInfo.usage = memoryUsage;
	//	allocInfo.flags = flags;

	//	VK_ASSERT(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, (VkImage*)&result._InternalImageID, (VmaAllocation*)&result._DeviceMemory, nullptr));

	//	if (initialLayout != ImageLayout::None) {
	//		VkCommandBuffer cb = VK_CONTEXT.GetPrimaryCommandPool()->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//		VkImageMemoryBarrier barrier{};
	//		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//		barrier.newLayout = Utils::ImageLayoutToVulkanImageLayout(initialLayout);
	//		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//		barrier.image = (VkImage)result._InternalImageID;

	//		if (Utils::IsDepthFormat(result.ImageFormat)) {
	//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	//			if (result.ImageFormat == Format::Depth24Stencil8) {
	//				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	//			}
	//		}
	//		else {
	//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//		}

	//		barrier.subresourceRange.baseMipLevel = 0;
	//		barrier.subresourceRange.levelCount = result.Levels;
	//		barrier.subresourceRange.baseArrayLayer = 0;
	//		barrier.subresourceRange.layerCount = result.Layers;

	//		VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	//		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	//		vkCmdPipelineBarrier(
	//			cb,
	//			sourceStage, destinationStage,
	//			0,
	//			0, nullptr,
	//			0, nullptr,
	//			1, &barrier
	//		);

	//		VK_CONTEXT.GetPrimaryCommandPool()->EndSingleTimeCommands(cb, VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue").Queue);
	//	}

	//	return result;*/
	//}
	//ImageSpecification VulkanAllocator::AllocateImage(const ImageAllocateSpecification& spec)
	//{
	//	//VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	//	//imageInfo.imageType = spec.Type;
	//	//imageInfo.format = Utils::FormatToVulkanFormat(spec.ImageFormat);
	//	//imageInfo.extent.width = spec.Width;
	//	//imageInfo.extent.height = spec.Height;
	//	//imageInfo.extent.depth = spec.Depth;
	//	//
	//	//imageInfo.mipLevels = spec.Levels;
	//	//imageInfo.arrayLayers = spec.Layers;
	//	//
	//	//imageInfo.samples = spec.Samples;
	//	//imageInfo.tiling = spec.Tiling;
	//	//
	//	//imageInfo.usage = spec.ImageUsage;
	//	//imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//	//imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	//
	//	//imageInfo.flags = spec.VulkanFlags;
	//	//
	//	//VmaAllocationCreateInfo allocationInfo{};
	//	//allocationInfo.flags = spec.VmaFlags;
	//	//allocationInfo.usage = spec.MemoryUsage;
	//	//
	//	//VkImage img{};
	//	//VmaAllocation allocation{};
	//	//VK_ASSERT(vmaCreateImage(m_Allocator, &imageInfo, &allocationInfo, &img, &allocation, nullptr));
	//	//
	//	//ImageSpecification image{};
	//	//image.Width = spec.Width;
	//	//image.Height = spec.Height;
	//	//image.Depth = spec.Depth;
	//	//image.ImageFormat = spec.ImageFormat;
	//	//image.Layers = spec.Layers;
	//	//image.Layout = spec.InitialLayout;
	//	//image.Levels = spec.Levels;
	//	//image._InternalImageID = (RendererID)img;
	//	//image._DeviceMemory = (RendererID)allocation;
	//	//image.IntendedLayout = spec.InitialLayout;
	//	//
	//	//return image;
	//	return {};
	//}
}
