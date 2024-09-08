#include "KDpch.h"
#include "VulkanTexture.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specs)
		:m_Specification(specs)
	{
		KD_CORE_ASSERT(specs.Type == ImageType::_2D);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(specs.Layers == 1);
		KD_CORE_ASSERT(!specs.IsCube);

		auto& backend = VK_CONTEXT.GetBackend();

		VulkanBackend::TextureInputInfo info{};
		info.Format = Utils::FormatToVulkanFormat(specs.Format);
		info.Aspects = Utils::IsDepthFormat(specs.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		info.Width = specs.Width;
		info.Height = specs.Height;
		info.Depth = 1;
		info.Layers = 1;
		info.Mips = specs.Mips;
		info.Type = VK_IMAGE_TYPE_2D;
		info.Samples = (VkSampleCountFlagBits)specs.Samples;
		info.Usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.ViewFormat = info.Format;
		info.ViewType = VK_IMAGE_VIEW_TYPE_2D;
		info.Swizzles[0] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
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

			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });


			for (auto& initData : specs.InitialDatas) {
				
				VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(initData.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);
				
				uint8_t* ptr = backend->BufferMap(stagingBuffer);
				std::memcpy(ptr, initData.Data, initData.Size);
				backend->BufferUnmap(stagingBuffer);
				
				VkBufferImageCopy copy{};
				copy.bufferImageHeight = 0;
				copy.bufferOffset = 0;
				copy.bufferRowLength = 0;
				copy.imageExtent = { specs.Width - initData.Width, specs.Height - initData.Height, specs.Depth - initData.Depth };
				copy.imageOffset = { (int)initData.Width, (int)initData.Height, (int)initData.Depth };
				copy.imageSubresource.aspectMask = info.Aspects;
				copy.imageSubresource.baseArrayLayer = 0;
				copy.imageSubresource.layerCount = 1;
				copy.imageSubresource.mipLevel = 0;

				backend->CommandCopyBufferToTexture(cb, stagingBuffer, m_Info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { copy });

				stagingBuffers.push_back(stagingBuffer);
			}
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
		info.Swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
		info.Swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
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

			backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });


			for (auto& initData : specs.InitialDatas) {

				VulkanBackend::BufferInfo stagingBuffer = backend->CreateBuffer(initData.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false);

				uint8_t* ptr = backend->BufferMap(stagingBuffer);
				std::memcpy(ptr, initData.Data, initData.Size);
				backend->BufferUnmap(stagingBuffer);

				VkBufferImageCopy copy{};
				copy.bufferImageHeight = 0;
				copy.bufferOffset = 0;
				copy.bufferRowLength = 0;
				copy.imageExtent = { specs.Width - initData.Width, specs.Height - initData.Height, specs.Depth - initData.Depth };
				copy.imageOffset = { (int)initData.Width, (int)initData.Height, (int)initData.Depth };
				copy.imageSubresource.aspectMask = info.Aspects;
				copy.imageSubresource.baseArrayLayer = initData.Layer;
				copy.imageSubresource.layerCount = 1;
				copy.imageSubresource.mipLevel = 0;

				backend->CommandCopyBufferToTexture(cb, stagingBuffer, m_Info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { copy });

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
	VulkanFramebufferTexture::VulkanFramebufferTexture(uint32_t width, uint32_t height, Format format, TextureSamples samples, bool isDepth)
		:m_Specification(ImageType::_2D)
	{
		m_Specification.Format = format;
		m_Specification.Width = width;
		m_Specification.Height = height;
		m_Specification.Depth = 1;
		m_Specification.Layers = 1;
		m_Specification.Mips = 1;
		m_Specification.Layout = isDepth ? ImageLayout::DepthAttachmentOptimal : ImageLayout::ColorAttachmentOptimal;
		m_Specification.IsCube = false;
		m_Specification.IsCpuReadable = false;
		m_Specification.Samples = samples;
		m_Specification.Swizzles[0] = TextureSwizzle::Red;
		m_Specification.Swizzles[1] = TextureSwizzle::Green;
		m_Specification.Swizzles[2] = TextureSwizzle::Blue;
		m_Specification.Swizzles[3] = TextureSwizzle::Alpha;

		auto& backend = VK_CONTEXT.GetBackend();

		VulkanBackend::TextureInputInfo info{};
		info.Format = Utils::FormatToVulkanFormat(format);
		info.Aspects = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		info.Width = width;
		info.Height = height;
		info.Depth = 1;
		info.Layers = 1;
		info.Mips = 1;
		info.Type = VK_IMAGE_TYPE_2D;
		info.Samples = (VkSampleCountFlagBits)samples;
		info.Usage = 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
			VK_IMAGE_USAGE_SAMPLED_BIT		|
			(isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		info.ViewFormat = info.Format;
		info.ViewType = VK_IMAGE_VIEW_TYPE_2D;
		info.Swizzles[0] = VK_COMPONENT_SWIZZLE_R;
		info.Swizzles[1] = VK_COMPONENT_SWIZZLE_G;
		info.Swizzles[2] = VK_COMPONENT_SWIZZLE_B;
		info.Swizzles[3] = VK_COMPONENT_SWIZZLE_A;
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
		barrier.subresourceRange.layerCount = 1;
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
}
