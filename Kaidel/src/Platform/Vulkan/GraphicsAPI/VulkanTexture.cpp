#include "KDpch.h"
#include "VulkanTexture.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Utils {
		static VulkanBackend::BufferInfo AddCopyOperation(
			const TextureData& data, const VulkanBackend::TextureInfo& info,
			VkCommandBuffer cb, Format format) {
			
			auto& backend = VK_BACKEND;
			
			uint32_t areaWidth = glm::min(info.ImageInfo.extent.width - data.OffsetX, data.Width);
			uint32_t areaHeight = glm::min(info.ImageInfo.extent.height - data.OffsetY, data.Height);
			uint32_t areaDepth = glm::min(info.ImageInfo.extent.depth - data.OffsetZ, data.Depth);
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
			barrier.subresourceRange.layerCount = info.ImageInfo.arrayLayers;

			return barrier;
		}

		static void GenerateMips(const VulkanBackend::TextureInfo& info, VkCommandBuffer cb, 
			VkImageLayout srcLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStages) {
			
			auto& backend = VK_BACKEND;
			backend->CommandBeginMarker(cb, "Mips generation");
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
				blit.srcSubresource.layerCount = info.ImageInfo.arrayLayers;
				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { w > 1 ? w / 2 : 1, h > 1 ? h / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = info.ImageInfo.arrayLayers;

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

			backend->CommandEndMarker(cb);
		}

		template<typename T>
		static VulkanBackend::TextureInfo CreateTexture(T& specs, VkImageType imageType, VkImageViewType viewType, bool isCube = false)
		{
			uint32_t mips = specs.Mipped ? (uint32_t)std::floor(std::log2(std::max(specs.Width, specs.Height))) + 1 : 1;

			auto& backend = VK_BACKEND;
	

			VulkanBackend::TextureInputInfo inputInfo{};
			inputInfo.Format = Utils::FormatToVulkanFormat(specs.Format);
			inputInfo.Aspects = Utils::IsDepthFormat(specs.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			inputInfo.Width = specs.Width;
			inputInfo.Height = specs.Height;
			inputInfo.Depth = specs.Depth;
			inputInfo.Layers = isCube ? 6 : specs.Layers;
			inputInfo.Mips = mips;
			inputInfo.Type = imageType;
			inputInfo.Samples = (VkSampleCountFlagBits)specs.Samples;
			inputInfo.Usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			inputInfo.ViewFormat = inputInfo.Format;
			inputInfo.ViewType = viewType;
			inputInfo.Swizzles[0] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[0]);
			inputInfo.Swizzles[1] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[1]);
			inputInfo.Swizzles[2] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[2]);
			inputInfo.Swizzles[3] = Utils::TextureSwizzleToVulkanComponentSwizzle(specs.Swizzles[3]);
			inputInfo.IsCube = isCube;
			inputInfo.IsCpuReadable = specs.IsCpuReadable;

			specs.Mips = mips;

			VulkanBackend::TextureInfo info = backend->CreateTexture(inputInfo);

			std::vector<VulkanBackend::BufferInfo> stagingBuffers;
			if constexpr (!std::is_same_v<T, TextureAtlasSpecification>)
			{
				stagingBuffers.reserve(specs.InitialDatas.size());
			}
			VK_CONTEXT.SubmitSingle(VK_TRANSFER_Q,
				[&](VkCommandBuffer cb)
				{
					VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
					if constexpr (!std::is_same_v<T, TextureAtlasSpecification>)
					{
						if (!specs.InitialDatas.empty()) {
								layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

								VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
								barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
								barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
								barrier.srcAccessMask = 0;
								barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
								barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
								barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
								barrier.image = info.ViewInfo.image;
								barrier.subresourceRange = info.ViewInfo.subresourceRange;

								backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });

								for (auto& initData : specs.InitialDatas) {
									auto stagingBuffer = Utils::AddCopyOperation(initData, info, cb, specs.Format);
									stagingBuffers.push_back(stagingBuffer);
								}
								if (mips > 1)
									Utils::GenerateMips(info, cb, layout, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
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
						barrier.image = info.ViewInfo.image;
						barrier.subresourceRange = info.ViewInfo.subresourceRange;

						backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });
					}
				});

			for (auto& stagingBuffer : stagingBuffers) {
				backend->DestroyBuffer(stagingBuffer);
			}

			return info;
		}

	}

	//TODO: add assert so specs.Layout != None
	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& specs)
	{
		Init(specs);
	}
	VulkanTexture2D::~VulkanTexture2D()
	{
		KD_CORE_INFO("Texture {} Deleted", (void*)this);
		VK_CONTEXT.GetBackend()->DestroyTexture(m_Info);
	}

	void VulkanTexture2D::Init(const Texture2DSpecification& specs)
	{
		KD_CORE_ASSERT(specs.Type() == ImageType::_2D);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(specs.Layers == 1);
		KD_CORE_ASSERT(!specs.IsCube());

		m_Specification = specs;

		m_Info = Utils::CreateTexture(m_Specification, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D);
	}

	VulkanTextureLayered::VulkanTextureLayered(const TextureLayeredSpecification& specs)
		:m_Specification(specs)
	{
		KD_CORE_ASSERT(specs.Type() == ImageType::_2D_Array);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(!specs.IsCube());

		m_Info = Utils::CreateTexture(m_Specification, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
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

		VK_CONTEXT.SubmitSingle(VK_TRANSFER_Q,
		[&](VkCommandBuffer cb)
		{
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
		});
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
			specs.LayerCount,
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

	VulkanTextureCube::VulkanTextureCube(const TextureCubeSpecification& specs)
		: m_Specification(specs)
	{
		KD_CORE_ASSERT(specs.Type() == ImageType::_2D);
		KD_CORE_ASSERT(specs.Layout != ImageLayout::None);
		KD_CORE_ASSERT(specs.Depth == 1);
		KD_CORE_ASSERT(specs.Layers == 1);
		KD_CORE_ASSERT(specs.IsCube());

		m_Info = Utils::CreateTexture(m_Specification, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_CUBE, true);
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		VK_BACKEND->DestroyTexture(m_Info);
	}

	VulkanTextureAtlas::VulkanTextureAtlas(const TextureAtlasSpecification& specification)
		: m_Specification(specification), m_AtlasManager(specification.Width, specification.Height)
	{
		m_Info = Utils::CreateTexture(m_Specification, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D);
	}

	VulkanTextureAtlas::~VulkanTextureAtlas()
	{
		VK_BACKEND->DestroyTexture(m_Info);
	}

	const Kaidel::AtlasSamplingCoords& VulkanTextureAtlas::GetSamplingCoords(uint32_t i) const
	{
		return m_AtlasManager.GetAtlasSamplingCoords(i);
	}

	uint32_t VulkanTextureAtlas::AddToAtlas(const void* data, uint32_t width, uint32_t height)
	{
		AtlasRect rect{};
		rect.Width = width;
		rect.Height = height;

		uint32_t index = m_AtlasManager.AddToAtlas(rect);
		
		//Is atlas resized
		if (m_AtlasManager.GetWidth() != m_Specification.Width || m_AtlasManager.GetHeight() != m_Specification.Height)
		{
			m_Specification.Width = m_AtlasManager.GetWidth();
			m_Specification.Height = m_AtlasManager.GetHeight();

			VK_BACKEND->DestroyTexture(m_Info);

			m_Info = Utils::CreateTexture(m_Specification, VK_IMAGE_TYPE_2D, VK_IMAGE_VIEW_TYPE_2D);
		}

		auto& coords = m_AtlasManager.GetAtlasSamplingCoords(index);

		TextureData tData;
		tData.Data = data;
		tData.Width = width;
		tData.Height = height;
		tData.OffsetX = coords.BottomLeftPixel.x;
		tData.OffsetY = coords.BottomLeftPixel.y;

		SetData(tData, index);

		return index;
	}
	
	void VulkanTextureAtlas::SetData(const TextureData& data, uint32_t i)
	{
		const auto& samplingCoords = GetSamplingCoords(i);

		VulkanBackend::BufferInfo stagingBuffer;

		VK_CONTEXT.SubmitSingle(VK_TRANSFER_Q,
			[&](VkCommandBuffer cb)
			{
				auto& backend = VK_BACKEND;

				VkImageLayout layout = Utils::ImageLayoutToVulkanImageLayout(m_Specification.Layout);
				
				VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
				auto srcStages =
					Utils::PipelineStagesToVulkanPipelineStageFlags(Utils::LayoutToPipelineStages(m_Specification.Layout));


				if (layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				{
					barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					barrier.srcAccessMask = Utils::AccessFlagsToVulkanAccessFlags(Utils::LayoutToAccessFlags(m_Specification.Layout));
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.oldLayout = layout;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.image = m_Info.ViewInfo.image;
					barrier.subresourceRange = m_Info.ViewInfo.subresourceRange;

					backend->CommandPipelineBarrier(cb, srcStages, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, { barrier });
				}

				stagingBuffer = Utils::AddCopyOperation(data, m_Info, cb, m_Specification.Format);

				if (layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout != VK_IMAGE_LAYOUT_UNDEFINED) {

					std::swap(barrier.srcAccessMask, barrier.dstAccessMask);
					std::swap(barrier.oldLayout, barrier.newLayout);

					backend->CommandPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, srcStages, {}, {}, { barrier });
				}
			});

		VK_BACKEND->DestroyBuffer(stagingBuffer);
	}

}
