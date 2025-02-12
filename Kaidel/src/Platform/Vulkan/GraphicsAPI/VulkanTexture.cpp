#include "KDpch.h"
#include "VulkanTexture.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Utils {
		static VulkanBackend::BufferInfo AddCopyOperation(
			const TextureData& data, const VulkanBackend::TextureInfo& info,
			VkCommandBuffer cb, Format format) {
			
			auto& backend = VK_BACKEND;
			
			uint32_t areaWidth = info.ImageInfo.extent.width - data.OffsetX;
			uint32_t areaHeight = info.ImageInfo.extent.height - data.OffsetY;
			uint32_t areaDepth = info.ImageInfo.extent.depth - data.OffsetZ;
			uint64_t areaSize = Utils::CalculateImageSize(areaWidth, areaHeight, areaDepth, 1, Utils::CalculatePixelSize(format));

			VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(areaSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);

			uint8_t* ptr = backend->BufferMap(stagingBuffer);
			std::memcpy(ptr, data.Data, areaSize);
			backend->BufferUnmap(stagingBuffer);

			VkBufferImageCopy copy{};
			copy.bufferImageHeight = 0;
			copy.bufferOffset = 0;
			copy.bufferRowLength = 0;
			copy.imageExtent = { areaWidth,areaHeight,areaDepth };
			copy.imageOffset = { (int)data.OffsetX,(int)data.OffsetY,(int)data.OffsetZ };
			copy.imageSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
			copy.imageSubresource.baseArrayLayer = data.Layer;
			copy.imageSubresource.layerCount = 1;
			copy.imageSubresource.mipLevel = 0;

			backend->CommandCopyBufferToTexture(cb, stagingBuffer, info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { copy });

			return stagingBuffer;
		}

		static VkImageMemoryBarrier TransitionMipBarrier(const VulkanBackend::TextureInfo& info, uint32_t mip,
			VkImageLayout srcLayout, VkAccessFlags srcAccess,
			VkImageLayout dstLayout, VkAccessFlags dstAccess) {
			
			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;
			barrier.oldLayout = srcLayout;
			barrier.newLayout = dstLayout;
			barrier.image = info.ViewInfo.image;
			barrier.subresourceRange = info.ViewInfo.subresourceRange;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseMipLevel = mip;

			return barrier;
		}

		static void GenerateMips(const VulkanBackend::TextureInfo& info, VkCommandBuffer cb, 
			VkImageLayout srcLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStages) {
			auto& backend = VK_BACKEND;
			
			uint32_t mipLevels = info.ImageInfo.mipLevels;

			int w = info.ImageInfo.extent.width, h = info.ImageInfo.extent.height;

			VkImageLayout layout = srcLayout;
			VkAccessFlags access = srcAccess;
			VkPipelineStageFlags stages = srcStages;

			if (srcLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
				VkImageMemoryBarrier barrier = 
					TransitionMipBarrier(info, 0, srcLayout, srcAccess, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT);
				backend->CommandPipelineBarrier(
					cb,
					srcStages,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					{}, {}, { barrier }
				);
			}

			for (uint32_t i = 1; i < mipLevels; ++i) {

				//Transform mip i to dst.
				{
					VkImageMemoryBarrier barrier = 
						TransitionMipBarrier(
							info,i,
							VK_IMAGE_LAYOUT_UNDEFINED,0,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_ACCESS_TRANSFER_WRITE_BIT);
					backend->CommandPipelineBarrier(
						cb,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						{},
						{},
						{ barrier });
				}
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { w, h, 1 };
				blit.srcSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { w > 1 ? h / 2 : 1, h > 1 ? h / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				backend->CommandBlitTexture(
					cb,
					info, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					blit);

				//Transform mip i to src for next iteration.
				{
					VkImageMemoryBarrier barrier =
						TransitionMipBarrier(
							info, i, 
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, 
							VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT);
					backend->CommandPipelineBarrier(
						cb,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						{}, {}, { barrier }
					);
				}

				if(w > 1)
					w /= 2;
				if(h > 1)
					h /= 2;
			}

			//reverse all mips to layout.

			if (srcLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
				for (uint32_t i = 0; i < mipLevels; ++i) {
					//Transform mip i to layout.
					{
						VkImageMemoryBarrier barrier =
							TransitionMipBarrier(info, i,
								VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
								srcLayout, srcAccess);
						backend->CommandPipelineBarrier(
							cb,
							VK_PIPELINE_STAGE_TRANSFER_BIT,
							srcStages,
							{},
							{},
							{ barrier });
					}
				}
			}

		}


	}


	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specs)
		:m_Specification(specs)
	{
		KD_CORE_ASSERT(specs.Type == ImageType::_2D);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(specs.Layers == 1);
		KD_CORE_ASSERT(!specs.IsCube);

		uint32_t mips = (uint32_t)std::floor(std::log2(std::max(specs.Width, specs.Height))) + 1;

		auto& backend = VK_CONTEXT.GetBackend();

		VulkanBackend::TextureInputInfo info{};
		info.Format = Utils::FormatToVulkanFormat(specs.Format);
		info.Aspects = Utils::IsDepthFormat(specs.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		info.Width = specs.Width;
		info.Height = specs.Height;
		info.Depth = 1;
		info.Layers = 1;
		info.Mips = mips;
		info.Type = VK_IMAGE_TYPE_2D;
		info.Samples = (VkSampleCountFlagBits)specs.Samples;
		info.Usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.ViewFormat = info.Format;
		info.ViewType = VK_IMAGE_VIEW_TYPE_2D;
		info.Swizzles[0] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[1]);
		info.Swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[2]);
		info.Swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[3]);
		info.IsCube = false;
		info.IsCpuReadable = specs.IsCpuReadable;

		m_Specification.Mips = mips;

		m_Info = backend->CreateTexture(info);

		VkCommandBuffer cb = backend->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());
		backend->CommandBufferBegin(cb);

		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		std::vector<VulkanBackend::BufferInfo> stagingBuffers;
		
		if (specs.InitialDatas.size()) {
			layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.image = m_Info.ViewInfo.image;
			barrier.subresourceRange = m_Info.ViewInfo.subresourceRange;

			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });

			for (auto& initData : specs.InitialDatas) {
				auto stagingBuffer = Utils::AddCopyOperation(initData, m_Info, cb, specs.Format);
				stagingBuffers.push_back(stagingBuffer);
			}
			if(mips > 1)
				Utils::GenerateMips(m_Info,cb, layout,VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		}

		VkImageLayout finalLayout = Utils::ImageLayoutToVulkanImageLayout(specs.Layout);
		if(finalLayout != layout){
			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = layout;
			barrier.newLayout = finalLayout;
			barrier.image = m_Info.ViewInfo.image;
			barrier.subresourceRange.aspectMask = info.Aspects;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.subresourceRange.levelCount = mips;

			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });
		}

		backend->CommandBufferEnd(cb);

		backend->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(), { cb }, VK_CONTEXT.GetSingleSubmitFence());
		backend->FenceWait(VK_CONTEXT.GetSingleSubmitFence());

		backend->DestroyCommandBuffer(cb, VK_CONTEXT.GetPrimaryCommandPool());

		for (auto& stagingBuffer : stagingBuffers) {
			backend->DestroyBuffer(stagingBuffer);
		}
	}
	VulkanTexture2D::~VulkanTexture2D()
	{
		VK_CONTEXT.GetBackend()->DestroyTexture(m_Info);
	}
	VulkanTextureLayered::VulkanTextureLayered(const TextureLayeredSpecification& specs)
		:m_Specification(specs)
	{
		KD_CORE_ASSERT(specs.Type == ImageType::_1D_Array || specs.Type == ImageType::_2D_Array);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(!specs.IsCube);

		auto& backend = VK_CONTEXT.GetBackend();

		VulkanBackend::TextureInputInfo info{};
		info.Format = Utils::FormatToVulkanFormat(specs.Format);
		info.Aspects = Utils::IsDepthFormat(specs.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		info.Width = specs.Width;
		info.Height = specs.Height;
		info.Depth = 1;
		info.Layers = specs.Layers;
		info.Mips = specs.Mips;
		info.Type = specs.Type == ImageType::_1D_Array ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D;
		info.Samples = (VkSampleCountFlagBits)specs.Samples;
		info.Usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.ViewFormat = info.Format;
		info.ViewType = specs.Type == ImageType::_1D_Array ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		info.Swizzles[0] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[1]);
		info.Swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[2]);
		info.Swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[3]);
		info.IsCube = false;
		info.IsCpuReadable = specs.IsCpuReadable;

		m_Info = backend->CreateTexture(info);

		VkCommandBuffer cb = backend->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());
		backend->CommandBufferBegin(cb);

		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		std::vector<VulkanBackend::BufferInfo> stagingBuffers;

		if (specs.InitialDatas.size()) {
			layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.image = m_Info.ViewInfo.image;
			barrier.subresourceRange = m_Info.ViewInfo.subresourceRange;

			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });

			for (auto& initData : specs.InitialDatas) {
				auto stagingBuffer = Utils::AddCopyOperation(initData, m_Info, cb, specs.Format);
				stagingBuffers.push_back(stagingBuffer);
			}
		}

		VkImageLayout finalLayout = Utils::ImageLayoutToVulkanImageLayout(specs.Layout);
		if (finalLayout != layout) {
			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.oldLayout = layout;
			barrier.newLayout = finalLayout;
			barrier.image = m_Info.ViewInfo.image;
			barrier.subresourceRange.aspectMask = info.Aspects;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.layerCount = specs.Layers;
			barrier.subresourceRange.levelCount = specs.Mips;
			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });
		}
		backend->CommandBufferEnd(cb);
		
		backend->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(), { cb }, VK_CONTEXT.GetSingleSubmitFence());
		backend->FenceWait(VK_CONTEXT.GetSingleSubmitFence());

		backend->DestroyCommandBuffer(cb, VK_CONTEXT.GetPrimaryCommandPool());

		for (auto& stagingBuffer : stagingBuffers) {
			backend->DestroyBuffer(stagingBuffer);
		}

	}
	VulkanTextureLayered::~VulkanTextureLayered()
	{
		VK_CONTEXT.GetBackend()->DestroyTexture(m_Info);
	}
	VulkanFramebufferTexture::VulkanFramebufferTexture(uint32_t width, uint32_t height, uint32_t layers, Format format, TextureSamples samples, bool isDepth)
		:m_Specification(ImageType::_2D)
	{
		int formatChannelCount = Utils::CalculateChannelCount(format);

		m_Specification.Format = format;
		m_Specification.Width = width;
		m_Specification.Height = height;
		m_Specification.Depth = 1;
		m_Specification.Layers = layers;
		m_Specification.Mips = 1;
		m_Specification.Layout = isDepth ? ImageLayout::DepthAttachmentOptimal : ImageLayout::ColorAttachmentOptimal;
		m_Specification.IsCube = false;
		m_Specification.IsCpuReadable = false;
		m_Specification.Samples = samples;
		
		m_Specification.Swizzles[0] = TextureSwizzle::Red;
		m_Specification.Swizzles[1] = TextureSwizzle::Green;
		m_Specification.Swizzles[2] = TextureSwizzle::Blue;
		m_Specification.Swizzles[3] = TextureSwizzle::Alpha;

		//if (Utils::IsDepthFormat(format)) {
		//	m_Specification.Swizzles[0] = TextureSwizzle::Red;
		//	m_Specification.Swizzles[1] = TextureSwizzle::Red;
		//	m_Specification.Swizzles[2] = TextureSwizzle::Red;
		//	m_Specification.Swizzles[3] = TextureSwizzle::One;
		//}


		auto& backend = VK_CONTEXT.GetBackend();

		VulkanBackend::TextureInputInfo info{};
		info.Format = Utils::FormatToVulkanFormat(format);
		info.Aspects = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		info.Width = width;
		info.Height = height;
		info.Depth = 1;
		info.Layers = layers;
		info.Mips = 1;
		info.Type = VK_IMAGE_TYPE_2D;
		info.Samples = (VkSampleCountFlagBits)samples;
		info.Usage = 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
			VK_IMAGE_USAGE_SAMPLED_BIT		|
			(isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		info.ViewFormat = info.Format;
		info.ViewType = layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		
		info.Swizzles[0] = VK_COMPONENT_SWIZZLE_R;
		info.Swizzles[1] = VK_COMPONENT_SWIZZLE_G;
		info.Swizzles[2] = VK_COMPONENT_SWIZZLE_B;
		info.Swizzles[3] = VK_COMPONENT_SWIZZLE_A;

		//if (Utils::IsDepthFormat(format)) {
		//	info.Swizzles[0] = VK_COMPONENT_SWIZZLE_R;
		//	info.Swizzles[1] = VK_COMPONENT_SWIZZLE_R;
		//	info.Swizzles[2] = VK_COMPONENT_SWIZZLE_R;
		//	info.Swizzles[3] = VK_COMPONENT_SWIZZLE_ONE;
		//}

		
		info.IsCube = false;
		info.IsCpuReadable = false;

		m_Info = backend->CreateTexture(info);

		VkCommandBuffer cb = backend->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());

		backend->CommandBufferBegin(cb);

		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.image = m_Info.ViewInfo.image;
		barrier.subresourceRange.aspectMask = info.Aspects;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.layerCount = layers;
		barrier.subresourceRange.levelCount = 1;

		backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });
		
		backend->CommandBufferEnd(cb);
		
		backend->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(), { cb }, VK_CONTEXT.GetSingleSubmitFence());
		backend->FenceWait(VK_CONTEXT.GetSingleSubmitFence());

		backend->DestroyCommandBuffer(cb, VK_CONTEXT.GetPrimaryCommandPool());
	}
	VulkanFramebufferTexture::~VulkanFramebufferTexture()
	{
		VK_BACKEND->DestroyTexture(m_Info);
	}
	VulkanTextureReference::VulkanTextureReference(const TextureReferenceSpecification& specs)
		: m_Specification(specs)
	{
		auto& backend = VK_BACKEND;

		const VulkanBackend::TextureInfo* referenceInfo = (const VulkanBackend::TextureInfo*)specs.Reference->GetBackendInfo();
		auto& referenceSpecs = specs.Reference->GetTextureSpecification();

		KD_CORE_ASSERT(specs.LayerCount > 0);

		VkImageViewType viewType;
		if (specs.LayerCount > 1) {
			viewType = referenceInfo->ViewInfo.viewType;
		}
		else if (specs.LayerCount == 1) {
			if (referenceInfo->ViewInfo.viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) {
				viewType = VK_IMAGE_VIEW_TYPE_1D;
			}
			else if (referenceInfo->ViewInfo.viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
				viewType = VK_IMAGE_VIEW_TYPE_2D;
			}
			else {
				viewType = referenceInfo->ViewInfo.viewType;
			}
		}

		VkComponentSwizzle swizzles[4] = { VK_COMPONENT_SWIZZLE_MAX_ENUM };
		swizzles[0] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[1]);
		swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[2]);
		swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[3]);

		m_Info = backend->CreateTextureFromExisting(
			referenceInfo->ViewInfo.image,
			specs.StartLayer,
			specs.StartLayer,
			specs.StartMip,
			specs.MipCount,
			referenceInfo->ViewInfo.format,
			referenceInfo->ViewInfo.subresourceRange.aspectMask,
			viewType,
			swizzles
		);
	}
	VulkanTextureReference::~VulkanTextureReference()
	{
		VK_BACKEND->DestroyTexture(m_Info);
	}
}
