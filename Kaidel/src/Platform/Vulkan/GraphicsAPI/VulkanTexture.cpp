#include "KDpch.h"
#include "Kaidel/Core/Application.h"
#include "VulkanTexture.h"
#include "VulkanGraphicsContext.h"
#include "VulkanTransferBuffer.h"
#include "Kaidel/Renderer/RenderCommand.h"

namespace Kaidel {
	
	namespace Utils {
		static VkImageView CreateImageView(Format textureFormat, uint32_t levels, uint32_t layers, VkImage image,/*R,G,B,A*/ TextureSwizzle swizzles[4]) {
			VkImageView view{};
			VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.viewType = layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.subresourceRange.aspectMask = Utils::IsDepthFormat(textureFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.layerCount = layers;
			viewInfo.subresourceRange.levelCount = levels;
			viewInfo.image = image;
			viewInfo.format = Utils::FormatToVulkanFormat(textureFormat);
			viewInfo.components.r = Utils::TextureSwizzleToVulkanComponentSwizzle(swizzles[0]);
			viewInfo.components.g = Utils::TextureSwizzleToVulkanComponentSwizzle(swizzles[1]);
			viewInfo.components.b = Utils::TextureSwizzleToVulkanComponentSwizzle(swizzles[2]);
			viewInfo.components.a = Utils::TextureSwizzleToVulkanComponentSwizzle(swizzles[3]);

			VK_ASSERT(vkCreateImageView(VK_DEVICE.GetDevice(), &viewInfo, nullptr, &view));
			return view;
		}


	}
	
	#pragma region Texture2D

	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& spec)
		:m_Specification(spec)
	{

		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		VkSampleCountFlags samples = Utils::TextureSamplesToVulkanSampleCountFlags(m_Specification.Samples);
		uint32_t mips = m_Specification.MipMaps;
		Format format = m_Specification.TextureFormat;
		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		VkImageTiling tiling = (spec.Usage & TextureUsage_CPUReadable) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;

		VkImageUsageFlags usageFlags = Utils::TextureUsageToVulkanImageUsageFlags(m_Specification.Usage);
		uint32_t vmaFlags = (spec.Usage & TextureUsage_CPUReadable) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;

		TextureSwizzle swizzles[4] = { m_Specification.SwizzleR,m_Specification.SwizzleG ,m_Specification.SwizzleB ,m_Specification.SwizzleA };

		ImageAllocateSpecification allocation{};
		allocation.Width = width;
		allocation.Height = height;
		allocation.Depth = 1;
		allocation.ImageFormat = format;
		allocation.Samples = (VkSampleCountFlagBits)samples;
		allocation.Levels = mips;
		allocation.MemoryUsage = memUsage;
		allocation.Tiling = tiling;
		allocation.ImageUsage = usageFlags;
		allocation.VmaFlags = vmaFlags;
		allocation.InitialLayout = ImageLayout::None;
		allocation.Layers = 1;
		allocation.VulkanFlags = 0;
		allocation.Type = VK_IMAGE_TYPE_2D;

		m_Image = VK_ALLOCATOR.AllocateImage(allocation);

		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &m_AllocationInfo);

		if ((usageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) && m_Specification.InitialData != nullptr) {
			//Only for mip 0.
			uint32_t imageSize = Utils::CalculateImageSize(width, height, 1, 1, Utils::CalculatePixelSize(format));
			VulkanBuffer vulkanBuffer = VK_ALLOCATOR.AllocateBuffer(imageSize, VMA_MEMORY_USAGE_CPU_ONLY, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

			vmaCopyMemoryToAllocation(VK_ALLOCATOR.GetAllocator(), m_Specification.InitialData, vulkanBuffer.Allocation, 0, imageSize);

			VkBuffer buffer = vulkanBuffer.Buffer;
			VkImage image = (VkImage)m_Image._InternalImageID;
			VkCommandBuffer commandBuffer = VK_CONTEXT.GetPrimaryCommandPool()->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			VkBufferImageCopy copy{};
			copy.bufferOffset = 0;
			copy.imageOffset = { 0,0,0 };
			copy.imageExtent = { width,height,1 };
			copy.imageSubresource.aspectMask = Utils::IsDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			copy.imageSubresource.baseArrayLayer = 0;
			copy.imageSubresource.layerCount = 1;
			copy.imageSubresource.mipLevel = 0;
			copy.bufferRowLength = 0;
			copy.bufferImageHeight = 0;


			 // 1. Transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL  
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Assuming it's undefined initially  
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange.aspectMask = Utils::IsDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier
			);

			vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copy
			);

			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier
			);

			VK_CONTEXT.GetPrimaryCommandPool()->EndSingleTimeCommands(commandBuffer, VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue").Queue);

			VK_ALLOCATOR.DestroyBuffer(vulkanBuffer);
		}
		
		m_Image.ImageView = (RendererID)Utils::CreateImageView(format, mips, 1, (VkImage)m_Image._InternalImageID, swizzles);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		vkDestroyImageView(VK_DEVICE.GetDevice(), (VkImageView)m_Image.ImageView, nullptr);
		VK_ALLOCATOR.DestroyImage(m_Image);
	}

	void* VulkanTexture2D::Map(uint32_t mipMap) const
	{
		VkImageAspectFlags aspect = 0;
		if (Utils::IsDepthFormat(m_Image.ImageFormat)) {
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (m_Image.ImageFormat == Format::Depth24Stencil8) {
				aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		}


		VkImageSubresource subres = {};
		subres.aspectMask = aspect;
		subres.arrayLayer = 0;
		subres.mipLevel = mipMap;

		VkSubresourceLayout subresLayout = {};
		vkGetImageSubresourceLayout(VK_DEVICE.GetDevice(), (VkImage)m_Image._InternalImageID, &subres, &subresLayout);

		void* ptr = nullptr;

		VmaAllocationInfo info{};

		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &info);


		VkResult err = vkMapMemory(
			VK_DEVICE.GetDevice(),
			info.deviceMemory,
			info.offset + subresLayout.offset,
			subresLayout.size,
			0,
			&ptr);

		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &ptr);
		return ptr;
	}

	void VulkanTexture2D::Unmap() const
	{
		VmaAllocationInfo info{};

		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &info);
		vkUnmapMemory(VK_DEVICE.GetDevice(), info.deviceMemory);
	}
	#pragma endregion

	#pragma region TextureLayered2D

	VulkanTextureLayered2D::VulkanTextureLayered2D(const TextureLayered2DSpecification& spec)
		:m_Specification(spec)
	{
		uint32_t width = m_Specification.InitialWidth;
		uint32_t height = m_Specification.InitialHeight;
		VkSampleCountFlags samples = Utils::TextureSamplesToVulkanSampleCountFlags(m_Specification.Samples);
		uint32_t mips = 1;
		Format format = m_Specification.TextureFormat;
		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		VkImageTiling tiling = (m_Specification.Usage & TextureUsage_CPUReadable) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;

		VkImageUsageFlags usageFlags = Utils::TextureUsageToVulkanImageUsageFlags(m_Specification.Usage);
		uint32_t vmaFlags = (m_Specification.Usage & TextureUsage_CPUReadable) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;

		TextureSwizzle swizzles[4] = { m_Specification.SwizzleR,m_Specification.SwizzleG ,m_Specification.SwizzleB ,m_Specification.SwizzleA };

		ImageAllocateSpecification allocation{};
		allocation.Width = width;
		allocation.Height = height;
		allocation.Depth = 1;
		allocation.ImageFormat = format;
		allocation.Samples = (VkSampleCountFlagBits)samples;
		allocation.Levels = mips;
		allocation.MemoryUsage = memUsage;
		allocation.Tiling = tiling;
		allocation.ImageUsage = usageFlags;
		allocation.VmaFlags = vmaFlags;
		allocation.InitialLayout = ImageLayout::None;
		allocation.Layers = 2;
		allocation.VulkanFlags = 0;
		allocation.Type = VK_IMAGE_TYPE_2D;

		m_Image = VK_ALLOCATOR.AllocateImage(allocation);
		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &m_AllocationInfo);

		m_Image.ImageView = (RendererID)Utils::CreateImageView(format, mips, 2, (VkImage)m_Image._InternalImageID, swizzles);
	}

	VulkanTextureLayered2D::~VulkanTextureLayered2D()
	{
		DeleteImage();
	}

	void* VulkanTextureLayered2D::Map(uint32_t mipMap, uint32_t layer) const
	{
		VkImageAspectFlags aspect = 0;
		if (Utils::IsDepthFormat(m_Image.ImageFormat)) {
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (m_Image.ImageFormat == Format::Depth24Stencil8) {
				aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		}


		VkImageSubresource subres = {};
		subres.aspectMask = aspect;
		subres.arrayLayer = layer;
		subres.mipLevel = mipMap;

		VkSubresourceLayout subresLayout = {};
		vkGetImageSubresourceLayout(VK_DEVICE.GetDevice(), (VkImage)m_Image._InternalImageID, &subres, &subresLayout);

		void* ptr = nullptr;

		VmaAllocationInfo info{};

		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &info);


		VkResult err = vkMapMemory(
			VK_DEVICE.GetDevice(),
			info.deviceMemory,
			info.offset + subresLayout.offset,
			subresLayout.size,
			0,
			&ptr);

		vmaMapMemory(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &ptr);
		return ptr;
	}

	void VulkanTextureLayered2D::Unmap() const
	{
		VmaAllocationInfo info{};

		vmaGetAllocationInfo(VK_ALLOCATOR.GetAllocator(), (VmaAllocation)m_Image._DeviceMemory, &info);
		vkUnmapMemory(VK_DEVICE.GetDevice(), info.deviceMemory);
	}

	void VulkanTextureLayered2D::Copy(VkCommandBuffer cmd, Ref<TransferBuffer> src, const BufferToTextureCopyRegion& region)
	{
		Image& dst = m_Image;

		bool needsTransition = dst.Layout != ImageLayout::TransferDstOptimal;
		ImageLayout oldLayout = dst.Layout;
		oldLayout = oldLayout == ImageLayout::None ? ImageLayout::ShaderReadOnlyOptimal : oldLayout;
		Ref<VulkanTransferBuffer> vulkanSrcBuffer = src;

		if (needsTransition)
			Utils::Transition(cmd, dst, ImageLayout::TransferDstOptimal);
		//Copy
		VkBufferImageCopy copyRegion = {};

		copyRegion.bufferOffset = region.BufferOffset;

		copyRegion.imageOffset.x = region.TextureOffset.x;
		copyRegion.imageOffset.y = region.TextureOffset.y;
		copyRegion.imageOffset.z = region.TextureOffset.z;

		copyRegion.imageExtent.width = region.TextureRegionSize.x;
		copyRegion.imageExtent.height = region.TextureRegionSize.y;
		copyRegion.imageExtent.depth = region.TextureRegionSize.z;

		copyRegion.imageSubresource.aspectMask = Utils::GetAspectFlags(dst.ImageFormat);
		copyRegion.imageSubresource.mipLevel = region.Mipmap;
		copyRegion.imageSubresource.baseArrayLayer = region.StartLayer;
		copyRegion.imageSubresource.layerCount = region.LayerCount;

		vkCmdCopyBufferToImage(
			cmd,
			vulkanSrcBuffer->GetBuffer().Buffer,
			(VkImage)dst._InternalImageID,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		//Revert transition
		if (needsTransition)
			Utils::Transition(cmd, dst, oldLayout);
	}

	void VulkanTextureLayered2D::CopyData(void* data,uint32_t width, uint32_t height, uint64_t size, uint32_t layer)
	{
		Ref<TransferBuffer> src = TransferBuffer::Create(size, data, size);
		Application::Get().SubmitToMainThread([src,width,height,layer,this]() {
			vkDeviceWaitIdle(VK_DEVICE.GetDevice());

			SCOPED_TIMER(fmt::format("Push to layer {} without reallocation", layer));
			BufferToTextureCopyRegion region{};
			region.BufferOffset = 0;
			region.LayerCount = 1;
			region.Mipmap = 0;
			region.StartLayer = layer;
			region.TextureOffset = { 0,0,0 };
			region.TextureRegionSize = { width,height,1 };


			VkCommandBuffer cmd = VK_CONTEXT.GetPrimaryCommandPool()->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			Copy(cmd, src, region);

			VK_CONTEXT.GetPrimaryCommandPool()->EndSingleTimeCommands(cmd, VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue"));
		});
	}


	void VulkanTextureLayered2D::Reallocate()
	{
		uint32_t width = m_Specification.InitialWidth;
		uint32_t height = m_Specification.InitialHeight;
		VkSampleCountFlags samples = Utils::TextureSamplesToVulkanSampleCountFlags(m_Specification.Samples);
		uint32_t mips = 1;
		Format format = m_Specification.TextureFormat;
		VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		VkImageTiling tiling = (m_Specification.Usage & TextureUsage_CPUReadable) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;

		VkImageUsageFlags usageFlags = Utils::TextureUsageToVulkanImageUsageFlags(m_Specification.Usage);
		uint32_t vmaFlags = (m_Specification.Usage & TextureUsage_CPUReadable) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;

		TextureSwizzle swizzles[4] = { m_Specification.SwizzleR,m_Specification.SwizzleG ,m_Specification.SwizzleB ,m_Specification.SwizzleA };
		
		Image newImage{};
		//Create the new image.
		{
			

			ImageAllocateSpecification allocation{};
			allocation.Width = m_Image.Width;
			allocation.Height = m_Image.Height;
			allocation.Depth = 1;
			allocation.ImageFormat = m_Image.ImageFormat;
			allocation.Samples = (VkSampleCountFlagBits)samples;
			allocation.Levels = mips;
			allocation.MemoryUsage = memUsage;
			allocation.Tiling = tiling;
			allocation.ImageUsage = usageFlags;
			allocation.VmaFlags = vmaFlags;
			allocation.InitialLayout = ImageLayout::None;
			allocation.Layers = m_Image.Layers * 1.5 + 1;
			allocation.VulkanFlags = 0;
			allocation.Type = VK_IMAGE_TYPE_2D;
			
			newImage = VK_ALLOCATOR.AllocateImage(allocation);
		}

		//Copy the old image to the new image.
		{
			VkCommandBuffer cmd = VK_CONTEXT.GetPrimaryCommandPool()->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			ImageLayout oldLayout = m_Image.Layout;
			oldLayout = oldLayout == ImageLayout::None ? ImageLayout::ShaderReadOnlyOptimal : oldLayout;

			Utils::Transition(cmd, m_Image, ImageLayout::TransferSrcOptimal);


			Utils::Transition(cmd, newImage, ImageLayout::TransferDstOptimal);

			VkImageCopy copy{};

			copy.srcOffset = { 0,0,0 };
			copy.dstOffset = { 0,0,0 };

			copy.extent = { m_Image.Width, m_Image.Height, 1 };

			copy.srcSubresource.aspectMask = Utils::GetAspectFlags(m_Image.ImageFormat);
			copy.srcSubresource.baseArrayLayer = 0;
			copy.srcSubresource.mipLevel = 0;
			copy.srcSubresource.layerCount = m_Image.Layers;

			copy.dstSubresource.aspectMask = Utils::GetAspectFlags(m_Image.ImageFormat);
			copy.dstSubresource.baseArrayLayer = 0;
			copy.dstSubresource.mipLevel = 0;
			copy.dstSubresource.layerCount = m_Image.Layers;

			vkCmdCopyImage(
				cmd,
				(VkImage)m_Image._InternalImageID,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				(VkImage)newImage._InternalImageID,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copy);

			Utils::Transition(cmd, newImage, oldLayout);
			
			VK_CONTEXT.GetPrimaryCommandPool()->EndSingleTimeCommands(cmd, VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue"));
		}

		//Delete the old image.
		DeleteImage();

		m_Image = newImage;
		m_Image.ImageView = (RendererID)Utils::CreateImageView(format, mips, 2, (VkImage)m_Image._InternalImageID, swizzles);
	}

	void VulkanTextureLayered2D::ReallocateAndCopy(void* data, uint32_t width, uint32_t height, uint64_t size, uint32_t layer)
	{
		Ref<TransferBuffer> src = TransferBuffer::Create(size, data, size);
		Application::Get().SubmitToMainThread([=]() {
			vkDeviceWaitIdle(VK_DEVICE.GetDevice());
			
			{
				SCOPED_TIMER("Reallocation");

				Reallocate();
			}

			SCOPED_TIMER(fmt::format("Push to layer {} with reallocation", layer));

			BufferToTextureCopyRegion region{};
			region.BufferOffset = 0;
			region.LayerCount = 1;
			region.Mipmap = 0;
			region.StartLayer = layer;
			region.TextureOffset = { 0,0,0 };
			region.TextureRegionSize = { width,height,1 };

			VkCommandBuffer cmd = VK_CONTEXT.GetPrimaryCommandPool()->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			Copy(cmd, src, region);

			VK_CONTEXT.GetPrimaryCommandPool()->EndSingleTimeCommands(cmd, VK_PHYSICAL_DEVICE.GetQueue("GraphicsQueue"));
		});
	}

	void VulkanTextureLayered2D::DeleteImage()
	{
		vkDestroyImageView(VK_DEVICE.GetDevice(), (VkImageView)m_Image.ImageView, nullptr);
		VK_ALLOCATOR.DestroyImage(m_Image);
	}

	uint32_t VulkanTextureLayered2D::Push(const TextureLayered2DLayerSpecification& layerSpec)
	{
		KD_CORE_ASSERT(layerSpec.Width <= m_Specification.InitialHeight && layerSpec.Height <= m_Specification.InitialHeight);
		KD_CORE_ASSERT((m_Specification.Usage & TextureUsage_Updateable) || (m_Specification.Usage & TextureUsage_CopyTo));
		
		uint32_t pushIndex = m_LayerSpecifications.size();
		
		//Array has space
		if (pushIndex < m_Image.Layers) {
			//Data setting requested
			if (layerSpec.InitialData != nullptr) {
				uint64_t size = Utils::CalculateImageSize(layerSpec.Width, layerSpec.Height, 1, 1, Utils::CalculatePixelSize(m_Specification.TextureFormat));
				CopyData(layerSpec.InitialData, layerSpec.Width, layerSpec.Height, size, pushIndex);
			}
		}
		//Array has no space
		else {
			uint64_t size = Utils::CalculateImageSize(layerSpec.Width, layerSpec.Height, 1, 1, Utils::CalculatePixelSize(m_Specification.TextureFormat));
			ReallocateAndCopy(layerSpec.InitialData, layerSpec.Width, layerSpec.Height, size, pushIndex);
		}

		m_LayerSpecifications.push_back(layerSpec);
		return pushIndex;
	}

	#pragma endregion
}
